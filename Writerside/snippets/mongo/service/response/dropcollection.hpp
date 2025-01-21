//
// Created by Rakesh on 18/12/2024.
//

#pragma once

#if defined __has_include
  #if __has_include("../../../common/visit_struct/visit_struct_intrusive.hpp")
    #include "../../../common/visit_struct/visit_struct_intrusive.hpp"
    #include "../../../common/util/serialise.hpp"
  #else
    #include <mongo-service/common/visit_struct/visit_struct_intrusive.hpp>
    #include <mongo-service/common/util/serialise.hpp>
  #endif
#endif

#include <string>

namespace spt::mongoservice::api::model::response
{
  struct DropCollection
  {
    explicit DropCollection( bsoncxx::document::view document ) { util::unmarshall( *this, document ); }
    DropCollection() = default;
    ~DropCollection() = default;
    DropCollection(DropCollection&&) = default;
    DropCollection& operator=(DropCollection&&) = default;

    DropCollection(const DropCollection&) = delete;
    DropCollection& operator=(const DropCollection&) = delete;

    BEGIN_VISITABLES(DropCollection);
    VISITABLE_DIRECT_INIT(bool, dropCollection, {false});
    END_VISITABLES;
  };
}
