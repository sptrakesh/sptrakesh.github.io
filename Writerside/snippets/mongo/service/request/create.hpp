//
// Created by Rakesh on 13/12/2024.
//

#pragma once

#include "action.hpp"
#include "../../options/insert.hpp"

#if defined __has_include
  #if __has_include("../../../common/util/json.hpp")
    #include "../../../common/util/json.hpp"
  #else
    #include <mongo-service/common/util/json.hpp>
  #endif
#endif

#include <functional>
#include <bsoncxx/builder/stream/document.hpp>

namespace spt::mongoservice::api::model::request
{
  template <util::Visitable Document, util::Visitable Metadata>
  requires std::is_same_v<decltype(Document::id), bsoncxx::oid> && std::constructible_from<Document, bsoncxx::document::view>
  struct Create
  {
    Create() = default;
    ~Create() = default;
    Create(Create&&) = default;
    Create& operator=(Create&&) = default;

    Create(const Create&) = delete;
    Create& operator=(const Create&) = delete;

    BEGIN_VISITABLES(Create);
    Document document;
    VISITABLE(std::optional<Metadata>, metadata);
    VISITABLE(std::optional<options::Insert>, options);
    std::string database;
    std::string collection;
    std::string application;
    std::string correlationId;
    Action action{Action::create};
    bool skipVersion{false};
    bool skipMetric{false};
    END_VISITABLES;
  };

  template <util::Visitable Document, util::Visitable Metadata>
  requires std::is_same_v<decltype(Document::id), bsoncxx::oid> && std::constructible_from<Document, bsoncxx::document::view>
  struct CreateWithReference
  {
    explicit CreateWithReference( const Document& doc ) : document{ std::cref( doc ) } {}
    CreateWithReference( const Document& doc, const Metadata& md ) : document{ std::cref( doc ) }, metadata{ std::cref( md ) } {}
    ~CreateWithReference() = default;
    CreateWithReference(CreateWithReference&&) = default;
    CreateWithReference& operator=(CreateWithReference&&) = default;

    CreateWithReference(const CreateWithReference&) = delete;
    CreateWithReference& operator=(const CreateWithReference&) = delete;

    BEGIN_VISITABLES(CreateWithReference);
    std::reference_wrapper<const Document> document;
    VISITABLE(std::optional<std::reference_wrapper<const Metadata>>, metadata);
    VISITABLE(std::optional<options::Insert>, options);
    std::string database;
    std::string collection;
    std::string application;
    std::string correlationId;
    Action action{Action::create};
    bool skipVersion{false};
    bool skipMetric{false};
    END_VISITABLES;
  };
}