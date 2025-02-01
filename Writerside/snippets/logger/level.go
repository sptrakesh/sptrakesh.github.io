package logger

import (
  "fmt"
  "log"
  "strings"
  "time"
)

type Level int

const (
  Severe Level = iota
  Warning Level = iota
  Info Level = iota
  Fine Level = iota
  Finer Level = iota
)

func LevelFromString(l string) Level {
  lc := strings.ToLower(l)

  switch lc {
  case "severe": return Severe
  case "warning": return Warning
  case "info": return Info
  case "fine": return Fine
  default: return Finer
  }
}

func (l Level) String() string {
  levels := [...]string{"SEVERE", "WARNING", "INFO", "FINE", "FINER"}

  if l < Severe || l > Finer { return "UNKNOWN" }
  return levels[l]
}

func (l *Logger) Fatal(m *Message) {
  log.Fatal(m.String())
}

func (l *Logger) Severe(m *Message) {
  if l.level >= Severe {
    if _, err := fmt.Fprintf(l.file, "%v [%v] %v\n", time.Now().Format(time.RFC3339), Severe.String(), m.String()); err != nil {
      log.Printf("[%v] %v", Severe.String(), m.String())
    }
  }
}

func (l *Logger) Warning(m *Message) {
  if l.level >= Warning {
    if _, err := fmt.Fprintf(l.file, "%v [%v] %v\n", time.Now().Format(time.RFC3339), Warning.String(), m.String()); err != nil {
      log.Printf("[%v] %v", Warning.String(), m.String())
    }
  }
}

func (l *Logger) Info(m *Message) {
  if l.level >= Info {
    if _, err := fmt.Fprintf(l.file, "%v [%v] %v\n", time.Now().Format(time.RFC3339), Info.String(), m.String()); err != nil {
      log.Printf("[%v] %v", Info.String(), m.String())
    }
  }
}

func (l *Logger) Fine(m *Message) {
  if l.level >= Fine {
    if _, err := fmt.Fprintf(l.file, "%v [%v] %v\n", time.Now().Format(time.RFC3339), Fine.String(), m.String()); err != nil {
      log.Printf("[%v] %v", Fine.String(), m.String())
    }
  }
}

func (l *Logger) Finer(m *Message) {
  if l.level >= Finer {
    if _, err := fmt.Fprintf(l.file, "%v [%v] %v\n", time.Now().Format(time.RFC3339), Finer.String(), m.String()); err != nil {
      log.Printf("[%v] %v", Finer.String(), m.String())
    }
  }
}

func (l *Logger) Println(v ...interface{}) {
  if l == nil || l.file == nil {
    log.Println(v...)
    return
  }
  if _, err := fmt.Fprintf(l.file, "[negroni] %v\n", v); err != nil {
    log.Println(v...)
  }
}

func (l *Logger) Printf(format string, v ...interface{}) {
  if l == nil || l.file == nil {
    log.Printf(format, v...)
    return
  }
  if _, err := fmt.Fprintf(l.file, "[negroni] %v\n", fmt.Sprintf(format, v...)); err != nil {
    log.Printf(format, v...)
  }
}