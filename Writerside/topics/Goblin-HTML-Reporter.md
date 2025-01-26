# Goblin HTML Reporter
A simple Reporter implementation that generates **HTML** reports for **BDD** test suites written
using [goblin](https://github.com/franela/goblin).

## Options
The reporter supports the following options specified as command line arguments:
* `report.file` - The primary report output file. This file contains the full aggregated results
  of all tests that were executed. Default value if not specified is `$PWD/test-results/index.html`
* `report.preserve` - A flag used to indicate that report output should be preserved. If this flag
  is specified, a directory hierarchy will be created per test run under which all report files
  will be generated. The top level directory will still be the parent directory of the report 
  file specified (or default location). Under this parent directory, two additional levels of
  directories will be created:
  * `day` - A directory with the current day in ISO 8601 format - `yyyy-MM-dd`
  * `time` - Created under the day directory with current time - `hhmmss`

## Output Files
The reporter produces the following output files:
* `<directory>/index.html` - A simple index file that just lists the individual test results 
  files, and the *summary* and *results* files.
* `<directory>/summary.html` - Test result summary in tabular format.
* `<directory>/results.html` - The full aggregated results of the test suite. This is the 
  file that is specified as command line argument to control the output directory if the 
  default is not desired.
* `<directory>/<TestName>.html` - Report with results from executing the test.

## Running
The following simple suite shows how to setup and use the html reporter. It just requires 
two additional lines of code in the test file (comments Line 1 and Line 2). The rest is 
standard goblin BDD test suite.

You can specify the main output file (default `$PWD/test-results/index.html`) by specifying 
a flag.

```shell
go test -report.file=/tmp/test.html
```

or to specify the target output directory and desired report file name as well as to 
preserve all report files per run

```shell
go test -report.file=/tmp/test.html -report.preserve
```

or to preserve all report files per run, but use the default output directory

```shell
go test -report.preserve
```

Consolidated test output will be written to the specified (or default) html file. In 
addition, the reporter creates a `Test<Xxx>.html` for each test (`TestXxx`) under the 
parent directory (default `$PWD/test-results`) as well as a `summary.html` file.

## Sample Test
<code-block lang="Go" src="goblin/TestExample.go" collapsible="true"/>

## Sample summary report
<img src="goblin-summary.png" alt="Summary report"/>

### Sample individual test report
Using default location, this file is generated as `$PWD/test-results/TestAuth.html`

<img src="goblin-auth-test.png" alt="Summary report"/>

## Reporter Implementation
<code-block lang="Go" src="goblin/reporter.go" collapsible="true"/>
