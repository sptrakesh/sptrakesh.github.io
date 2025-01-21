//
// Created by Rakesh on 13/12/2024.
//

#pragma once

#include "../../options/distinct.hpp"
#include "action.hpp"

#include <bsoncxx/builder/stream/document.hpp>

namespace spt::mongoservice::api::model::request
{
  template <util::Visitable Document>
  struct Distinct
  {
    struct Payload
    {
      explicit Payload( Document&& document ) : filter{ std::forward<Document>( document ) } {}
      Payload() = default;
      ~Payload() = default;
      Payload(Payload&&) = default;
      Payload& operator=(Payload&&) = default;
      bool operator==(const Payload&) const = default;

      Payload(const Payload&) = delete;
      Payload& operator=(const Payload&) = delete;

      BEGIN_VISITABLES(Payload);
      VISITABLE(std::optional<Document>, filter);
      VISITABLE(std::string, field);
      END_VISITABLES;
    };

    explicit Distinct( Document&& document ) : document{ std::forward<Document>( document ) } {}
    Distinct() = default;
    ~Distinct() = default;
    Distinct(Distinct&&) = default;
    Distinct& operator=(Distinct&&) = default;

    Distinct(const Distinct&) = delete;
    Distinct& operator=(const Distinct&) = delete;

    BEGIN_VISITABLES(Distinct);
    VISITABLE(std::optional<Payload>, document);
    VISITABLE(std::optional<options::Distinct>, options);
    std::string database;
    std::string collection;
    std::string application;
    std::string correlationId;
    Action action{Action::distinct};
    bool skipMetric{false};
    END_VISITABLES;
  };
}
