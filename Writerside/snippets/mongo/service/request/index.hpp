//
// Created by Rakesh on 14/12/2024.
//

#pragma once

#include "../../options/index.hpp"
#include "action.hpp"

#include <bsoncxx/builder/stream/document.hpp>

namespace spt::mongoservice::api::model::request
{
  template <util::Visitable Document>
  struct Index
  {
    explicit Index( Document&& document ) : document{ std::forward<Document>( document ) } {}
    Index() = default;
    ~Index() = default;
    Index(Index&&) = default;
    Index& operator=(Index&&) = default;

    Index(const Index&) = delete;
    Index& operator=(const Index&) = delete;

    BEGIN_VISITABLES(Index);
    VISITABLE(std::optional<Document>, document);
    VISITABLE(std::optional<options::Index>, options);
    std::string database;
    std::string collection;
    std::string application;
    std::string correlationId;
    Action action{Action::index};
    bool skipMetric{false};
    END_VISITABLES;
  };
}
