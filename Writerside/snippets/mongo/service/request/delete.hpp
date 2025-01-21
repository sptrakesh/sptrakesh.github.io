//
// Created by Rakesh on 13/12/2024.
//

#pragma once

#include "../../options/delete.hpp"
#include "action.hpp"

#include <bsoncxx/builder/stream/document.hpp>

namespace spt::mongoservice::api::model::request
{
  template <util::Visitable Document, util::Visitable Metadata>
  requires std::constructible_from<Metadata, bsoncxx::document::view>
  struct Delete
  {
    explicit Delete( Document&& document ) : document{ std::forward<Document>( document ) } {}
    Delete() = default;
    ~Delete() = default;
    Delete(Delete&&) = default;
    Delete& operator=(Delete&&) = default;

    Delete(const Delete&) = delete;
    Delete& operator=(const Delete&) = delete;

    BEGIN_VISITABLES(Delete);
    VISITABLE(std::optional<Document>, document);
    VISITABLE(std::optional<options::Delete>, options);
    std::optional<Metadata> metadata;
    std::string database;
    std::string collection;
    std::string application;
    std::string correlationId;
    Action action{Action::_delete};
    bool skipMetric{false};
    END_VISITABLES;
  };
}
