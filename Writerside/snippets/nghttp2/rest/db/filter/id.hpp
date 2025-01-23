//
// Created by Rakesh on 13/01/2025.
//

#pragma once

#include "model/defaultoid.hpp"

#include <boost/json/object.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongo-service/common/visit_struct/visit_struct_intrusive.hpp>

namespace spt::http2::rest::db::filter
{
  struct Id
  {
    Id( bsoncxx::oid customer, bsoncxx::oid id ) : id{ id }, customer{ customer } {}
    explicit Id( bsoncxx::oid id ) : id{ id } {}
    Id() = default;
    ~Id() = default;
    Id(const Id&) = default;
    Id& operator=(const Id&) = default;
    Id(Id&&) = default;
    Id& operator=(Id&&) = default;

    BEGIN_VISITABLES(Id);
    VISITABLE_DIRECT_INIT(bsoncxx::oid, id, {model::DEFAULT_OID});
    std::optional<bsoncxx::oid> customer;
    END_VISITABLES;
  };

  inline void populate( const Id& filter, bsoncxx::builder::stream::document& builder )
  {
    if ( filter.customer ) builder << "customer._id" << *filter.customer;
  }

  inline void populate( const Id& filter, boost::json::object& object )
  {
    if ( filter.customer ) object.emplace( "customer.id", filter.customer->to_string() );
  }
}
