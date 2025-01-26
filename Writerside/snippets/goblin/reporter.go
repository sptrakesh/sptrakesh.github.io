package main

import (
  "flag"
  "fmt"
  "io"
  "log"
  "os"
  "path/filepath"
  "strings"
  "sync"
  "time"

  "github.com/franela/goblin"
)

type ReportStats struct {
  failed, passed, pending, excluded int
  executionTime, totalExecutionTime time.Duration
  describes []string
  failures []*goblin.Failure
  file *os.File
}

type StatsMap map[string]*ReportStats

type HtmlReporter struct {
  aggregateTestsPassed int
  aggregateExecutionTime time.Duration
  reporter goblin.Reporter
  file *os.File
  writer io.Writer
  current string
  tests StatsMap
}

func GetReporter() *HtmlReporter {
  ronce.Do(func() {
    flag.Parse()
    r, err := newHtmlReporter(*outfile)
    if err != nil { panic(err) }
    reporter = r
  })

  return reporter
}

func newHtmlReporter(fileName string) (*HtmlReporter, error) {
  fn := fileName
  dir := filepath.Dir(fn)
  err := os.MkdirAll(dir, 0744)
  if err != nil {
    return nil, fmt.Errorf("cannot make directories for new logfile: %s", err)
  }

  if *saveoutput {
    dir := filepath.Dir(fn)
    t := time.Now()
    day := fmt.Sprintf("%d-%02d-%02d", t.Year(), t.Month(), t.Day())
    tm := fmt.Sprintf("%02d%02d%02d", t.Hour(), t.Minute(), t.Second())

    dest := filepath.Join(dir, day, tm)
    err := os.MkdirAll(dest, 0744)
    if err != nil {
      return nil, fmt.Errorf("cannot make directories for new logfile: %s", err)
    }

    fn = filepath.Base(fn)
    fn = filepath.Join(dest, fn)
  }

  mode := os.FileMode(0644)
  f, err := os.OpenFile(fn, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, mode)
  if err != nil {
    return nil, fmt.Errorf("cannot open new test report file: %s", err)
  }

  log.Println("Writing test html results to file", fn)
  fancy := goblin.TerminalFancier{}
  rep := goblin.DetailedReporter{}
  rep.SetTextFancier(goblin.TextFancier(&fancy))

  if _, err := fmt.Fprint(f, header); err != nil {
    log.Println("Error writing html header to file", f, "\n", err)
  }

  return &HtmlReporter{reporter: goblin.Reporter(&rep), file: f}, nil
}

func (r *HtmlReporter) SetReporter(rep goblin.Reporter) {
  r.reporter = rep
}

func (r *HtmlReporter) newReportStats(name string) *ReportStats {
  dir := filepath.Dir(r.file.Name())
  of := fmt.Sprintf("%v/%v.html", dir, name)

  mode := os.FileMode(0644)
  f, err := os.OpenFile(of, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, mode)
  if err != nil {
    panic(fmt.Errorf("cannot open new test report file: %s", err))
  }

  h := strings.Replace(header, "Test Report", fmt.Sprintf("%v Report", name), 1)
  if _, err := fmt.Fprint(f, h); err != nil {
    log.Println("Error writing html header to file", f.Name(), "\n", err)
  }

  r.writer = io.MultiWriter(f, r.file)
  return &ReportStats{file: f}
}

func (r *HtmlReporter) RegisterTest(name string) {
  if len(r.current) > 0 {
    if err := r.tests[r.current].file.Close(); err != nil {
      log.Println("Error closing test file", r.tests[r.current].file.Name(), "\n", err)
    }
  }

  r.current = name
  if r.tests == nil { r.tests = StatsMap{} }
  r.tests[name] = r.newReportStats(name)

  if _, err := fmt.Fprintf(r.writer, "<h2>%v</h2>\n", name); err != nil {
    log.Println("Error registering test", name, "to file", r.file.Name(), "\n", err)
  }
}

func (r *HtmlReporter) Failure(failure *goblin.Failure) {
  r.reporter.Failure(failure)
  s := r.tests[r.current]
  s.failures = append(s.failures, failure)
}

func (r *HtmlReporter) BeginDescribe(name string) {
  r.reporter.BeginDescribe(name)
  r.tests[r.current].describes = append(r.tests[r.current].describes, name)
  l := len(r.tests[r.current].describes)

  msg := fmt.Sprintf("<li><strong>%v</strong>\n<ol>\n", name)
  if l == 1 { msg = fmt.Sprintf("<strong>%v</strong>\n<ol>\n", name) }

  if _, err := fmt.Fprint(r.writer, msg); err != nil {
    log.Println("Error writing Describe", name, "at level", l, "to file", r.file.Name(), "\n", err)
  }
}

func (r *HtmlReporter) EndDescribe() {
  r.reporter.EndDescribe()
  l := len(r.tests[r.current].describes)

  msg := "</ol>\n</li>\n"
  if l == 1 { msg = "</ol>\n" }

  if _, err := fmt.Fprint(r.writer, msg); err != nil {
    log.Println("Error writing EndDescribe at level", l, "to file", r.file.Name(), "\n", err)
  }

  r.tests[r.current].describes = r.tests[r.current].describes[:l-1]
}

func (r *HtmlReporter) ItTook(duration time.Duration) {
  r.reporter.ItTook(duration)
  r.tests[r.current].executionTime = duration
  r.tests[r.current].totalExecutionTime += duration
  r.aggregateExecutionTime += duration
}

func (r *HtmlReporter) ItFailed(name string) {
  r.reporter.ItFailed(name)
  r.tests[r.current].failed++
  if _, err := fmt.Fprintf(r.writer, "<li><span style='color: red'>&#x2717; %d) %v</span></li>\n", r.tests[r.current].failed, name); err != nil {
    log.Println("Error writing ItFailed", name, "at level", len(r.tests[r.current].describes), "to file", r.file.Name(), "\n", err)
  }
}

func (r *HtmlReporter) ItPassed(name string) {
  r.reporter.ItPassed(name)
  r.tests[r.current].passed++
  if _, err := fmt.Fprintf(r.writer, "<li><span style='color: green'>&#x2713;</span> <span style='color: gray'>%v (%d ms)</span></li>\n", name, r.tests[r.current].executionTime/time.Millisecond); err != nil {
    log.Println("Error writing ItPassed", name, "at level", len(r.tests[r.current].describes), "to file", r.file.Name(), "\n", err)
  }
}

func (r *HtmlReporter) ItIsPending(name string) {
  r.reporter.ItIsPending(name)
  r.tests[r.current].pending++
  if _, err := fmt.Fprintf(r.writer, "<li><span style='color: cyan'>- %v</span></li>\n", name); err != nil {
    log.Println("Error writing ItIsPending", name, "at level", len(r.tests[r.current].describes), "to file", r.file.Name(), "\n", err)
  }
}

func (r *HtmlReporter) ItIsExcluded(name string) {
  r.reporter.ItIsExcluded(name)
  r.tests[r.current].excluded++
  if _, err := fmt.Fprintf(r.writer, "<li><span style='color: yellow'>- %v</span></li>\n", name); err != nil {
    log.Println("Error writing ItIsExcluded", name, "at level", len(r.tests[r.current].describes), "to file", r.file.Name(), "\n", err)
  }
}

func (r *HtmlReporter) Begin() {
  r.reporter.Begin()
}

func (r *HtmlReporter) End() {
  r.reporter.End()
  r.aggregateTestsPassed += r.tests[r.current].passed
  r.writeSummaries()

  r.saveSummary()
  r.saveIndex()
}

func (r *HtmlReporter) writeSummaries() {
  comp := fmt.Sprintf("%d tests complete", r.tests[r.current].passed)
  t := fmt.Sprintf("(%d ms)", r.tests[r.current].totalExecutionTime/time.Millisecond)

  if _, err := fmt.Fprintf(r.writer, "<div><span style='color: green'>%v</span> <span style='color: gray'>%v</span></div>\n", comp, t); err != nil {
    log.Println("Error writing tests complete to file", r.file.Name(), "\n", err)
  }

  if r.tests[r.current].pending > 0 {
    pend := fmt.Sprintf("%d test(s) pending", r.tests[r.current].pending)
    if _, err := fmt.Fprintf(r.writer, "<div style='color: cyan'>%v</div>\n", pend); err != nil {
      log.Println("Error writing tests pending to file", r.file.Name(), "\n", err)
    }
  }

  if r.tests[r.current].excluded > 0 {
    excl := fmt.Sprintf("%d test(s) excluded", r.tests[r.current].excluded)
    if _, err := fmt.Fprintf(r.writer, "<div style='color: yellow'>%v</div>\n", excl); err != nil {
      log.Println("Error writing tests excluded to file", r.file.Name(), "\n", err)
    }
  }

  if len(r.tests[r.current].failures) > 0 {
    if _, err := fmt.Fprintf(r.writer, "<div style='color: red'>%d tests failed</div>\n", len(r.tests[r.current].failures)); err != nil {
      log.Println("Error writing tests excluded to file", r.file.Name(), "\n", err)
    }
  }

  if _, err := fmt.Fprint(r.writer, "<ol>\n"); err != nil {
    log.Println("Error writing ol tag to file", r.file.Name(), "\n", err)
  }
  for i := range r.tests[r.current].failures {
    if _, err := fmt.Fprintf(r.writer, "<li>%s:\n", r.tests[r.current].failures[i].TestName); err != nil {
      log.Println("Error writing", r.tests[r.current].failures[i].TestName, "to file", r.file.Name(), "\n", err)
    }
    if _, err := fmt.Fprintf(r.writer, "<div style='color: red'>%s</div>\n", r.tests[r.current].failures[i].Message); err != nil {
      log.Println("Error writing", r.tests[r.current].failures[i].Message, "to file", r.file.Name(), "\n", err)
    }

    for _, stackItem := range r.tests[r.current].failures[i].Stack {
      if _, err := fmt.Fprintf(r.writer, "<div style='color: gray'>&nbsp;&nbsp;%s</div>\n", stackItem); err != nil {
        log.Println("Error writing", stackItem, "to file", r.file.Name(), "\n", err)
      }
    }
    if _, err := fmt.Fprint(r.writer, "</li>\n"); err != nil {
      log.Println("Error writing ending li tag to file", r.file.Name(), "\n", err)
    }
  }
  if _, err := fmt.Fprint(r.writer, "</ol>\n"); err != nil {
    log.Println("Error writing ending ol tag to file", r.file.Name(), "\n", err)
  }

  comp = fmt.Sprintf("%d total tests complete", r.aggregateTestsPassed)
  t = fmt.Sprintf("(%d ms)", r.aggregateExecutionTime/time.Millisecond)
  if _, err := fmt.Fprintf(r.file, "<div><span style='color: green'>%v</span> <span style='color: gray'>%v</span></div>\n<hr/>\n", comp, t); err != nil {
    log.Println("Error writing aggregate tests complete to file", r.file.Name(), "\n", err)
  }

  closeTags := "</body>\n</html>\n"
  if _, err := fmt.Fprint(r.writer, closeTags); err != nil {
    log.Println("Error closing html tags in file", r.tests[r.current].file.Name(), "\n", err)
  }

  if _, err := r.file.Seek(int64(-1 * len(closeTags)), 1); err != nil {
    log.Println("Error rewinding closing html tags in file", r.file.Name(), "\n", err)
  }

  if err := r.file.Sync(); err != nil {
    log.Println("Error syncing file", r.file.Name(), "\n", err)
  }
}

func (r *HtmlReporter) saveSummary() {
  dir := filepath.Dir(r.file.Name())
  of := fmt.Sprintf("%v/summary.html", dir)

  mode := os.FileMode(0644)
  f, err := os.OpenFile(of, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, mode)
  if err != nil {
    log.Println("Cannot open new test summary file:", of, "\n", err)
    return
  }

  h := strings.Replace(header, "Test Report", "Test Report Summary", 1)
  if _, err := fmt.Fprint(f, h); err != nil {
    log.Println("Error writing header to file", f.Name(), "\n", err)
  }

  if _, err := fmt.Fprint(f, `
<table>
<thead>
<tr>
<th>Test Suite</th>
<th>Tests Passed</th>
<th>Tests Pending</th>
<th>Tests Failed</th>
<th>Total Time</th>
</tr>
</thead>
<tbody>
`); err != nil {
    log.Println("Error writing table header to file", f.Name(), "\n", err)
  }


  var tpa, tpe, tfa int
  var te time.Duration
  for s := range r.tests {
    if _, err := fmt.Fprintf(f, "<tr>\n<td>%v</td>\n", s); err != nil {
      log.Println("Error writing test name to file", f.Name(), "\n", err)
    }
    tpa += r.tests[s].passed
    if _, err := fmt.Fprintf(f, "<td>%d</td>\n", r.tests[s].passed); err != nil {
      log.Println("Error writing tests passed to file", f.Name(), "\n", err)
    }
    tpe += r.tests[s].pending
    if _, err := fmt.Fprintf(f, "<td>%d</td>\n", r.tests[s].pending); err != nil {
      log.Println("Error writing tests pending to file", f.Name(), "\n", err)
    }
    tfa += r.tests[s].failed
    if _, err := fmt.Fprintf(f, "<td>%d</td>\n", r.tests[s].failed); err != nil {
      log.Println("Error writing tests failed to file", f.Name(), "\n", err)
    }
    te += r.tests[s].totalExecutionTime
    if _, err := fmt.Fprintf(f, "<td>%d (ms)</td>\n</tr>\n", r.tests[s].totalExecutionTime/time.Millisecond); err != nil {
      log.Println("Error writing test execution time to file", f.Name(), "\n", err)
    }
  }

  if _, err := fmt.Fprint(f, "<tr>\n<td><strong>Total</strong></td>\n"); err != nil {
    log.Println("Error writing test summary column to file", f.Name(), "\n", err)
  }
  if _, err := fmt.Fprintf(f, "<td><strong>%d</strong></td>\n", tpa); err != nil {
    log.Println("Error writing total passed column to file", f.Name(), "\n", err)
  }
  if _, err := fmt.Fprintf(f, "<td><strong>%d</strong></td>\n", tpe); err != nil {
    log.Println("Error writing total pending column to file", f.Name(), "\n", err)
  }
  if _, err := fmt.Fprintf(f, "<td><strong>%d</strong></td>\n", tfa); err != nil {
    log.Println("Error writing total failed column to file", f.Name(), "\n", err)
  }
  if _, err := fmt.Fprintf(f, "<td><strong>%d (ms)</strong></td>\n</tr>\n", te/time.Millisecond); err != nil {
    log.Println("Error writing total time column to file", f.Name(), "\n", err)
  }

  if _, err := fmt.Fprint(f, `
</tbody>
</table>
</body>
</html>
`); err != nil {
    log.Println("Error closing summary file", f.Name(), "\n", err)
  }
}

func (r *HtmlReporter) saveIndex() {
  dir := filepath.Dir(r.file.Name())
  of := fmt.Sprintf("%v/index.html", dir)

  mode := os.FileMode(0644)
  f, err := os.OpenFile(of, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, mode)
  if err != nil {
    log.Println("Cannot open new test index file:", of, "\n", err)
    return
  }

  if _, err := fmt.Fprintf(f, `
<h2>Test Results</h2>
<div><a href='%v'>Aggregate Results</a></div>
<div><a href='summary.html'>Test Summary</a></div>
<div>Links to individual test results below...</div>
`, filepath.Base(r.file.Name())); err != nil {
    log.Println("Error writing results and summary links to file", f.Name(), "\n", err)
  }

  if _, err := fmt.Fprintf(f, "%v\n<ol>\n", header); err != nil {
    log.Println("Error writing ol tag to file", f.Name(), "\n", err)
  }

  for s := range r.tests {
    p := "<span style='color: green'>&#x2713;</span>"
    if r.tests[s].failed > 0 {
      p = "<span style='color: red'>&#x2717;</span>"
    } else if r.tests[s].pending > 0 {
      p = "<span style='color: cyan'>-</span>"
    }
    if _, err := fmt.Fprintf(f, "<li>%v <a href='%v.html'>%v</a></li>\n", p, s, s); err != nil {
      log.Println("Error writing text link to file", f.Name(), "\n", err)
    }
  }

  if _, err := fmt.Fprint(f, "</ol>\n</body>\n</html>\n"); err != nil {
    log.Println("Error writing closing tags to file", f.Name(), "\n", err)
  }
}

var (
  outfile = flag.String("report.file", "test-results/results.html", "Sets the aggregate html report output file")
  saveoutput = flag.Bool("report.preserve", false, "Preserve html report output files in a date-time based directory structure")
  reporter *HtmlReporter
  ronce sync.Once
)

const header = `<!DOCTYPE html>
<html lang="en">
<head>
<title>Test Report</title>
<style>
b, p, div, span, a
{
  font-family: -apple-system, BlinkMacSystemFont, georgia, serif;
  font-size: small;
}
h1, h2, h3
{
  font-family: -apple-system, BlinkMacSystemFont, georgia, serif;
}
hr
{
  color: #ff9900;
}
a:link, a:visited
{
  font-family: -apple-system, BlinkMacSystemFont, georgia, serif;
  text-decoration: none;
  cursor: auto;
}

table
{
  border: 1px solid #1C6EA4;
  background-color: #EEEEEE;
  width: 100%;
  text-align: left;
  border-collapse: collapse;
}

table td, table th
{
  border: 1px solid #AAAAAA;
  font-family: -apple-system, BlinkMacSystemFont, georgia, serif;
  padding: 3px 2px;
}

table tbody td
{
  font-family: -apple-system, BlinkMacSystemFont, georgia, serif;
  font-size: 13px;
}

table tr:nth-child(even)
{
  background: #D0E4F5;
}

table thead
{
  background: #1C6EA4;
  background: -moz-linear-gradient(top, #5592bb 0%, #327cad 66%, #1C6EA4 100%);
  background: -webkit-linear-gradient(top, #5592bb 0%, #327cad 66%, #1C6EA4 100%);
  background: linear-gradient(to bottom, #5592bb 0%, #327cad 66%, #1C6EA4 100%);
  border-bottom: 2px solid #444444;
}

table thead th
{
  font-size: 15px;
  font-weight: bold;
  color: #FFFFFF;
  border-left: 2px solid #D0E4F5;
}

table thead th:first-child
{
  border-left: none;
}

table tfoot
{
  font-size: 14px;
  font-weight: bold;
  color: #FFFFFF;
  background: #D0E4F5;
  background: -moz-linear-gradient(top, #dcebf7 0%, #d4e6f6 66%, #D0E4F5 100%);
  background: -webkit-linear-gradient(top, #dcebf7 0%, #d4e6f6 66%, #D0E4F5 100%);
  background: linear-gradient(to bottom, #dcebf7 0%, #d4e6f6 66%, #D0E4F5 100%);
  border-top: 2px solid #444444;
}

table tfoot td
{
  font-size: 14px;
}

table tfoot .links
{
  text-align: right;
}

table tfoot .links a
{
  display: inline-block;
  background: #1C6EA4;
  color: #FFFFFF;
  padding: 2px 8px;
  border-radius: 5px;
}

ol
{
  list-style: none;
  counter-reset: item;
}
li
{
  counter-increment: item;
  margin-bottom: 5px;
}
li:before
{
  margin-right: 10px;
  content: counter(item);
  background: lightblue;
  border-radius: 100%;
  color: white;
  width: 1.2em;
  text-align: center;
  display: inline-block;
}
</style>
</head>
<body>
`