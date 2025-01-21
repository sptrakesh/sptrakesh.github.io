//
// Created by Rakesh on 13/12/2024.
//

#pragma once

#include "../../options/count.hpp"
#include "action.hpp"

#include <bsoncxx/builder/stream/document.hpp>

namespace spt::mongoservice::api::model::request
{
  template <util::Visitable Document>
  struct Count
  {
    explicit Count( Document&& document ) : document{ std::forward<Document>( document ) } {}
    Count() = default;
    ~Count() = default;
    Count(Count&&) = default;
    Count& operator=(Count&&) = default;

    Count(const Count&) = delete;
    Count& operator=(const Count&) = delete;

    BEGIN_VISITABLES(Count);
    VISITABLE(std::optional<Document>, document);
    VISITABLE(std::optional<options::Count>, options);
    std::string database;
    std::string collection;
    std::string application;
    std::string correlationId;
    Action action{Action::count};
    bool skipMetric{false};
    END_VISITABLES;
  };
}
