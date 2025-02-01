package logger

import "fmt"

type Message struct {
  Package  string
  File     string
  Function string
  Text     string
}

func (m *Message) String() string {
  if len(m.File) > 0 {
    return fmt.Sprintf("%v.%v.%v - %v", m.Package, m.File, m.Function, m.Text)
  }

  return fmt.Sprintf("%v.%v - %v", m.Package, m.Function, m.Text)
}