//
// Created by Rakesh on 27/02/2021.
//

#pragma once

#include <vector>
#include <boost/json/object.hpp>
#include <mongo-service/common/util/json.hpp>
#include <mongo-service/common/visit_struct/visit_struct_intrusive.hpp>

namespace spt::http2::rest::model
{
  template <typename Model>
  struct Entities
  {
    explicit Entities( std::string_view json ) { spt::util::json::unmarshall( *this, json ); }

    Entities() = default;
    Entities(Entities&&)  noexcept = default;
    Entities& operator=(Entities&&) = default;
    ~Entities() = default;

    Entities(const Entities&) = delete;
    Entities& operator=(const Entities&) = delete;

    void populate( simdjson::ondemand::object& object )
    {
      FROM_JSON( total, object );
      FROM_JSON( page, object );
    }

    BEGIN_VISITABLES(Entities<Model>);
    VISITABLE(std::vector<Model>, entities);
    VISITABLE(std::string, next);
    int32_t total{ 0 };
    int32_t page{ 0 };
    END_VISITABLES;
  };

  template <typename Model>
  void populate( Entities<Model>& model, simdjson::ondemand::object& object )
  {
    model.populate( object );
  }

  template <typename Model>
  void populate( const Entities<Model>& model, boost::json::object& object )
  {
    object.emplace( "total", model.total );
    object.emplace( "page", model.page );
    if ( !object.contains( "entities" ) ) object.emplace( "entities", boost::json::array{} );
  }
}