//
// Created by Rakesh on 13/12/2024.
//

#pragma once

#include "../../options/find.hpp"
#include "action.hpp"

#include <bsoncxx/builder/stream/document.hpp>

namespace spt::mongoservice::api::model::request
{
  template <util::Visitable Document>
  struct Retrieve
  {
    explicit Retrieve( Document&& document ) : document{ std::forward<Document>( document ) } {}
    Retrieve() = default;
    ~Retrieve() = default;
    Retrieve(Retrieve&&) = default;
    Retrieve& operator=(Retrieve&&) = default;

    Retrieve(const Retrieve&) = delete;
    Retrieve& operator=(const Retrieve&) = delete;

    BEGIN_VISITABLES(Retrieve);
    VISITABLE(std::optional<Document>, document);
    VISITABLE(std::optional<options::Find>, options);
    std::string database;
    std::string collection;
    std::string application;
    std::string correlationId;
    Action action{Action::retrieve};
    bool skipMetric{false};
    END_VISITABLES;
  };
}
