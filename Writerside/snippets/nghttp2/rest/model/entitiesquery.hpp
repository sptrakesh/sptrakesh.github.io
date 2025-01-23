//
// Created by Rakesh on 17/02/2021.
//

#pragma once

#include <optional>
#include <string>

namespace spt::http2::rest::model
{
  struct EntitiesQuery
  {
    std::optional<std::string> after{ std::nullopt };
    int16_t limit{ 25 };
    bool descending{ false };
  };
}
