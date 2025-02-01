package logger

import (
  "compress/gzip"
  "errors"
  "fmt"
  "io"
  "io/ioutil"
  "os"
  "path/filepath"
  "sort"
  "strings"
  "sync"
  "syscall"
  "time"
)

// Modified from https://github.com/natefinch/lumberjack
type Logger struct {
  fileName string
  maxBackups int
  localTime bool
  compress bool
  level Level

  day time.Time
  file *os.File
  mfile sync.Mutex

  millCh chan bool
  startMill sync.Once
}

var AppLogger *Logger

func NewLogger(file string, max int, localTime bool, compress bool, level Level) *Logger {
  l := Logger{fileName: file, maxBackups: max, localTime: localTime, compress: compress, level: level}
  l.setDay()
  if AppLogger == nil { AppLogger = &l }
  return &l
}

func (l *Logger) Write(p []byte) (int, error) {
  l.mfile.Lock()
  defer l.mfile.Unlock()

  if l.file == nil {
    if err := l.openExistingOrNew(); err != nil {
      return 0, err
    }
  }

  now := time.Now()
  if !l.localTime {now = now.UTC()}

  if now.After(l.day) {
    if err := l.rotate(); err != nil {
      return 0, err
    }
  }

  n, err := l.file.Write(p)
  return n, err
}

func (l *Logger) Close() error {
  l.mfile.Lock()
  defer l.mfile.Unlock()
  return l.close()
}

func (l *Logger) close() error {
  if l.file == nil { return nil }

  err := l.file.Close()
  l.file = nil
  return err
}

func (l *Logger) rotate() error {
  if err := l.close(); err != nil { return err }
  if err := l.openNew(); err != nil { return err }
  l.setDay()
  l.mill()
  return nil
}

func (l *Logger) openNew() error {
  err := os.MkdirAll(l.dir(), 0744)
  if err != nil {
    return fmt.Errorf("cannot make directories for new logfile: %s", err)
  }

  name := l.filename()
  mode := os.FileMode(0644)
  info, err := os.Stat(name)
  if err == nil {
    mode = info.Mode()
    newname := backupName(name, l.localTime)
    if err := os.Rename(name, newname); err != nil {
      return fmt.Errorf("cannot rename log file: %s", err)
    }

    if err := chown(name, info); err != nil { return err }
  }

  f, err := os.OpenFile(name, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, mode)
  if err != nil {
    return fmt.Errorf("cannot open new logfile: %s", err)
  }

  l.file = f
  return nil
}

func backupName(name string, local bool) string {
  dir := filepath.Dir(name)
  filename := filepath.Base(name)
  ext := filepath.Ext(filename)
  prefix := filename[:len(filename)-len(ext)]

  t := time.Now()
  if !local { t = t.UTC() }
  t.Add(time.Hour * -24)

  timestamp := fmt.Sprintf("%d-%02d-%02d", t.Year(), t.Month(), t.Day())
  return filepath.Join(dir, fmt.Sprintf("%s-%s%s", prefix, timestamp, ext))
}

func (l *Logger) openExistingOrNew() error {
  l.mill()

  filename := l.filename()
  _, err := os.Stat(filename)
  if os.IsNotExist(err) { return l.openNew() }

  if err != nil { return fmt.Errorf("error getting log file info: %s", err) }

  now := time.Now()
  if !l.localTime {now = now.UTC()}

  if now.After(l.day) { return l.rotate() }

  file, err := os.OpenFile(filename, os.O_APPEND|os.O_WRONLY, 0644)
  if err != nil { return l.openNew() }
  l.file = file
  return nil
}

func (l *Logger) filename() string {
  if l.fileName != "" { return l.fileName }
  name := filepath.Base(os.Args[0]) + ".log"
  return filepath.Join(os.TempDir(), name)
}

func (l *Logger) millRunOnce() error {
  if l.maxBackups == 0 && !l.compress { return nil }

  files, err := l.oldLogFiles()
  if err != nil { return err }

  var compress, remove []logInfo

  if l.maxBackups > 0 && l.maxBackups < len(files) {
    preserved := make(map[string]bool)
    var remaining []logInfo
    for _, f := range files {
      fn := f.Name()
      if strings.HasSuffix(fn, compressSuffix) { fn = fn[:len(fn)-len(compressSuffix)] }
      preserved[fn] = true

      if len(preserved) > l.maxBackups {
        remove = append(remove, f)
      } else {
        remaining = append(remaining, f)
      }
    }
    files = remaining
  }

  if l.compress {
    for _, f := range files {
      if !strings.HasSuffix(f.Name(), compressSuffix) {
        compress = append(compress, f)
      }
    }
  }

  for _, f := range remove {
    errRemove := os.Remove(filepath.Join(l.dir(), f.Name()))
    if err == nil && errRemove != nil {
      err = errRemove
    }
  }

  for _, f := range compress {
    fn := filepath.Join(l.dir(), f.Name())
    errCompress := compressLogFile(fn, fn+compressSuffix)
    if err == nil && errCompress != nil {
      err = errCompress
    }
  }

  return err
}

func (l *Logger) millRun() {
  for range l.millCh { _ = l.millRunOnce() }
}

func (l *Logger) mill() {
  l.startMill.Do(func() {
    l.millCh = make(chan bool, 1)
    go l.millRun()
  })

  select {
  case l.millCh <- true:
  default:
  }
}

func (l *Logger) oldLogFiles() ([]logInfo, error) {
  files, err := ioutil.ReadDir(l.dir())
  if err != nil {
    return nil, fmt.Errorf("cannot read log file directory: %s", err)
  }

  var logFiles []logInfo

  prefix, ext := l.prefixAndExt()

  for _, f := range files {
    if f.IsDir() { continue }
    if t, err := l.timeFromName(f.Name(), prefix, ext); err == nil {
      logFiles = append(logFiles, logInfo{t, f})
      continue
    }
    if t, err := l.timeFromName(f.Name(), prefix, ext+compressSuffix); err == nil {
      logFiles = append(logFiles, logInfo{t, f})
      continue
    }
  }

  sort.Sort(byFormatTime(logFiles))

  return logFiles, nil
}

func (l *Logger) timeFromName(filename, prefix, ext string) (time.Time, error) {
  if !strings.HasPrefix(filename, prefix) {
    return time.Time{}, errors.New("mismatched prefix")
  }

  if !strings.HasSuffix(filename, ext) {
    return time.Time{}, errors.New("mismatched extension")
  }

  ts := filename[len(prefix) : len(filename)-len(ext)]
  return time.Parse("2006-01-02", ts)
}

func (l *Logger) dir() string { return filepath.Dir(l.filename()) }

func (l *Logger) prefixAndExt() (prefix, ext string) {
  filename := filepath.Base(l.filename())
  ext = filepath.Ext(filename)
  prefix = filename[:len(filename)-len(ext)] + "-"
  return prefix, ext
}

func (l *Logger) setDay() {
  now := time.Now().Add(time.Hour * 24)
  if l.localTime {
    l.day = time.Date(now.Year(), now.Month(), now.Day(), 0, 0, 0, 0, time.Local)
  } else {
    l.day = time.Date(now.Year(), now.Month(), now.Day(), 0, 0, 0, 0, time.UTC)
  }
}

func compressLogFile(src, dst string) (err error) {
  f, err := os.Open(src)
  if err != nil {
    return fmt.Errorf("failed to open log file: %v", err)
  }
  defer f.Close()

  fi, err := os.Stat(src)
  if err != nil {
    return fmt.Errorf("failed to stat log file: %v", err)
  }

  if err := chown(dst, fi); err != nil {
    return fmt.Errorf("failed to chown compressed log file: %v", err)
  }

  gzf, err := os.OpenFile(dst, os.O_CREATE|os.O_TRUNC|os.O_WRONLY, fi.Mode())
  if err != nil {
    return fmt.Errorf("failed to open compressed log file: %v", err)
  }
  defer gzf.Close()

  gz := gzip.NewWriter(gzf)

  defer func() {
    if err != nil {
      if e := os.Remove(dst); e != nil {
        err = e
      } else {
        err = fmt.Errorf("failed to compress log file: %v", err)
      }
    }
  }()

  if _, err := io.Copy(gz, f); err != nil {
    return err
  }
  if err := gz.Close(); err != nil {
    return err
  }
  if err := gzf.Close(); err != nil {
    return err
  }

  if err := f.Close(); err != nil {
    return err
  }
  if err := os.Remove(src); err != nil {
    return err
  }

  return nil
}

func chown(name string, info os.FileInfo) error {
  f, err := os.OpenFile(name, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, info.Mode())
  if err != nil { return err }
  if err := f.Close(); err != nil { return fmt.Errorf("cannot close file %v", f) }
  stat := info.Sys().(*syscall.Stat_t)
  return os.Chown(name, int(stat.Uid), int(stat.Gid))
}

type logInfo struct {
  timestamp time.Time
  os.FileInfo
}

// byFormatTime sorts by newest time formatted in the name.
type byFormatTime []logInfo

func (b byFormatTime) Less(i, j int) bool { return b[i].timestamp.After(b[j].timestamp) }

func (b byFormatTime) Swap(i, j int) { b[i], b[j] = b[j], b[i] }

func (b byFormatTime) Len() int { return len(b) }

// ensure we always implement io.WriteCloser
var _ io.WriteCloser = (*Logger)(nil)

const compressSuffix = ".gz"