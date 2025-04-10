# Mongo Service Shell

A simple shell application for submitting minified JSON payloads to the service.

I developed this shell to make it easier to interact with MongoDB from a server.
This obviates the need to look up the database connection string, credentials etc.
when we wish to access the database directly.  With this utility, I am able to
`shell` into a running service docker container, and start the `shell` when I
want to run queries/commands against the configured database.

Sample query submitted to the service via the shell running on my localhost.

<img src="mongo-service-shell.png" alt="Shell Application" thumbnail="true"/>

Showing the display from the help option to the shell.

<img src="mongo-service-shell-help.png" alt="Shell Help" thumbnail="true"/>