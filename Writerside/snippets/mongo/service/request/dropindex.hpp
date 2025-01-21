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
  struct DropIndex
  {
    struct Specification
    {
      explicit Specification( Document&& document ) : specification{ std::forward<Document>( document ) } {}
      explicit Specification( bsoncxx::document::view bson ) { util::unmarshall( *this, bson ); }
      Specification() = default;
      ~Specification() = default;
      Specification(Specification&&) = default;
      Specification& operator=(Specification&&) = default;
      bool operator==(const Specification&) const = default;

      Specification(const Specification&) = delete;
      Specification& operator=(const Specification&) = delete;
      BEGIN_VISITABLES(Specification);
      VISITABLE(std::string, name);
      VISITABLE(std::optional<Document>, specification);
      END_VISITABLES;
    };

    explicit DropIndex( Document&& document ) : document{ std::forward<Document>( document ) } {}
    DropIndex() = default;
    ~DropIndex() = default;
    DropIndex(DropIndex&&) = default;
    DropIndex& operator=(DropIndex&&) = default;

    DropIndex(const DropIndex&) = delete;
    DropIndex& operator=(const DropIndex&) = delete;

    BEGIN_VISITABLES(DropIndex);
    VISITABLE(std::optional<Specification>, document);
    VISITABLE(std::optional<options::Index>, options);
    std::string database;
    std::string collection;
    std::string application;
    std::string correlationId;
    Action action{Action::dropIndex};
    bool skipMetric{false};
    END_VISITABLES;
  };
}
