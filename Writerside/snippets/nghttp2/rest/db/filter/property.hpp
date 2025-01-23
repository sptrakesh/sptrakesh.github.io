//
// Created by Rakesh on 14/01/2025.
//

#pragma once

#include <boost/json/object.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongo-service/common/visit_struct/visit_struct_intrusive.hpp>
#include <mongo-service/common/util/json.hpp>

namespace spt::http2::rest::db::filter
{
  template <typename ValueType>
  struct Property
  {
    Property( std::string_view property, ValueType value, std::string_view customer = {} ) : property( property ), customer( customer ), value{ std::move( value ) } {}
    Property() = default;
    ~Property() = default;
    Property(Property&&) = default;
    Property& operator=(Property&&) = default;

    Property(const Property&) = delete;
    Property& operator=(const Property&) = delete;

    BEGIN_VISITABLES(Property);
    std::string property;
    std::string customer;
    ValueType value;
    END_VISITABLES;
  };

  template <typename ValueType>
  void populate( const Property<ValueType>& filter, bsoncxx::builder::stream::document& builder )
  {
    builder << filter.property << filter.value;
    if ( !filter.customer.empty() ) builder << "customer.code" << filter.customer;
  }

  template <typename ValueType>
  void populate( const Property<ValueType>& filter, boost::json::object& object )
  {
    object.emplace( filter.property, spt::util::json::json( filter.value ) );
    if ( !filter.customer.empty() ) object.emplace( "customer.code", filter.customer );
  }
}