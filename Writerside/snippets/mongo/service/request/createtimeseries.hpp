//
// CreateTimeseriesd by Rakesh on 13/12/2024.
//

#pragma once

#include "action.hpp"
#include "../../options/insert.hpp"

#include <bsoncxx/builder/stream/document.hpp>

namespace spt::mongoservice::api::model::request
{
  template <util::Visitable Document>
  struct CreateTimeseries
  {
    CreateTimeseries() = default;
    ~CreateTimeseries() = default;
    CreateTimeseries(CreateTimeseries&&) = default;
    CreateTimeseries& operator=(CreateTimeseries&&) = default;

    CreateTimeseries(const CreateTimeseries&) = delete;
    CreateTimeseries& operator=(const CreateTimeseries&) = delete;

    BEGIN_VISITABLES(CreateTimeseries);
    VISITABLE(Document, document);
    VISITABLE(std::optional<options::Insert>, options);
    std::string database;
    std::string collection;
    std::string application;
    std::string correlationId;
    Action action{Action::createTimeseries};
    bool skipMetric{false};
    END_VISITABLES;
  };
}