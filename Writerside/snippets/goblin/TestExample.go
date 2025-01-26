package main

import (
  "testing"

  "github.com/franela/goblin"
)

func TestAuth(t *testing.T) {
  g := goblin.Goblin(t)
  GetReporter().RegisterTest(t.Name()) // Line 1
  g.SetReporter(goblin.Reporter(GetReporter())) // Line 2

  var Describe = g.Describe
  var It = g.It
  var Assert = g.Assert

  Describe("Login and logout from API", func() {
    var token string

    It("Login with username/password credentials", func() {
      response, tk, errs := login()
      Assert(errs == nil).IsTrue("Endpoint returned errors")
      Assert(response.StatusCode).Equal(200)
      Assert(len(tk) > 0).IsTrue("Endpoint did not return token")
      token = tk
    })

    It("Logout the JWT Token", func() {
      response, errs := logout(token)
      Assert(errs == nil).IsTrue("Endpoint returned errors")
      Assert(response.StatusCode).Equal(200)
    })

    It("Cannot use API with deleted token", func() {
      response, _, errs := customerByCode(token, "test")
      Assert(errs == nil).IsTrue("Endpoint returned errors")
      Assert(response.StatusCode).Equal(403)
    })

    It("Cannot logout invalidated JWT Token", func() {
      response, errs := logout(token)
      Assert(errs == nil).IsTrue("Endpoint returned errors")
      Assert(response.StatusCode != 200).IsTrue("Unexpected response code when logging out invalidated token")
    })
  })
}