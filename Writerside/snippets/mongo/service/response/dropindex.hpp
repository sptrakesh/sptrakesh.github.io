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
  struct DropIndex
  {
    explicit DropIndex( bsoncxx::document::view document ) { util::unmarshall( *this, document ); }
    DropIndex() = default;
    ~DropIndex() = default;
    DropIndex(DropIndex&&) = default;
    DropIndex& operator=(DropIndex&&) = default;

    DropIndex(const DropIndex&) = delete;
    DropIndex& operator=(const DropIndex&) = delete;

    BEGIN_VISITABLES(DropIndex);
    VISITABLE_DIRECT_INIT(bool, dropIndex, {false});
    END_VISITABLES;
  };
}
