# Boost Beast Metrics
[Boost.Beast](http://www.boost.org/libs/beast) is a high performance low level library for
creating web services. Here I describe how I integrated [Prometheus](https://prometheus.io/) metrics 
into our Beast powered services.

In a nutshell, we used the core library from the [prometheus-cpp](https://github.com/jupp0r/prometheus-cpp)
project, and adapted the metrics endpoint to use Beast instead of [civet](https://github.com/civetweb/civetweb).
The push part can also be implemented in terms of Beast, but we have not needed to use the prometheus
push gateway yet.

## Steps
* Download the source bundle from the *prometheus-cpp* project, and copy the code (headers and 
  implementation) files from the core directory into your project.
* Use the code in the pull directory as reference, and implement the **Beast** endpoint that 
  serves the collected metrics.
* Instrument the other **Beast** services as appropriate.

## RegistryManager
A global registry for all metrics captured by the system.

<code-block lang="C++" src="beast/prometheus/registry.hpp" collapsible="true"/>

## Metrics Endpoint
Prometheus polls a user specified endpoint for the *metrics* the application wishes to publish.
The default target is `/metrics`, and we have also chosen to bind the *metrics* to the same resource.
The following function publishes the collected metrics when Prometheus polls the endpoint.

<code-block lang="C++" src="beast/prometheus/metrics.hpp" collapsible="true"/>

## Integration Test
A very rudimentary integration test for the metrics endpoint.

<code-block lang="C++" src="beast/prometheus/metrics.cpp" collapsible="true"/>
