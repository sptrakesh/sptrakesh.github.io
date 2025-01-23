//
// Created by Rakesh on 15/01/2025.
//

#pragma once

#include <chrono>
#include <boost/json/object.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongo-service/common/visit_struct/visit_struct_intrusive.hpp>

namespace spt::http2::rest::db::filter
{
  struct Between
  {
    Between() = default;
    ~Between() = default;
    Between(Between&&) = default;
    Between& operator=(Between&&) = default;

    Between(const Between&) = delete;
    Between& operator=(const Between&) = delete;

    BEGIN_VISITABLES(Between);
    std::string field;
    std::chrono::time_point<std::chrono::system_clock> from;
    std::chrono::time_point<std::chrono::system_clock> to;
    std::optional<std::chrono::time_point<std::chrono::system_clock>> after;
    std::optional<bool> ignore;
    bool descending{ false };
    END_VISITABLES;
  };

  void populate( const Between& filter, bsoncxx::builder::stream::document& builder );
  void populate( const Between& filter, boost::json::object& object );
}