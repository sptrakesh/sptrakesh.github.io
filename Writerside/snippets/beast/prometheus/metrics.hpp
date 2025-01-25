#pragma once

#include "error.hpp"
#include "registry.hpp"
#include "log/NanoLog.hpp"
#include "prometheus/text_serializer.h"

namespace spt::http::service
{
  template<typename Body, typename Allocator, typename Send>
  void metrics( http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send)
  {
    const auto path = std::string(req.target().data(), req.target().size());
    RegistryManager::getInstance().counters->Add(
        {{"component", "monitoring"}, {"path", path}}).Increment();

    const auto data = prometheus::TextSerializer().Serialize(
        RegistryManager::getInstance().registry.Collect());

    http::response<http::dynamic_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/plain");

    beast::ostream(res.body()) << data;
    res.prepare_payload();

    return send(std::move(res));
  }
}