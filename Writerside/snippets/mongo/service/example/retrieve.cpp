#include <mongo-service/api/repository/repository.hpp>

namespace example
{
  struct Document
  {
    explicit Document( bsoncxx::document::view bson ) { spt::util::unmarshall( *this, bson ); }
    Document() = default;
    ~Document() = default;
    Document(Document&&) = default;
    Document& operator=(Document&&) = default;
    bool operator==(const Document&) const = default;

    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    BEGIN_VISITABLES(Document);
    VISITABLE(bsoncxx::oid, id);
    VISITABLE(std::string, str);
    VISITABLE_DIRECT_INIT(spt::util::DateTimeMs, created, {std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() )});
    VISITABLE_DIRECT_INIT(int64_t, integer, {5});
    VISITABLE_DIRECT_INIT(double, floating, {10.345});
    VISITABLE_DIRECT_INIT(bool, boolean, {true});
    END_VISITABLES;
  };
}

int main()
{
  using namespace spt::mongoservice::api;

  {
    auto retrieve = model::request::Retrieve{ model::request::IdFilter{} };
    retrieve.database = database;
    retrieve.collection = collection;
    retrieve.document->id = oid;
    retrieve.options = options::Find{};
    retrieve.options->sort = document{} << "str" << 1 << "_id" << -1 << finalize;
    retrieve.options->maxTime = std::chrono::milliseconds{ 1000 };
    retrieve.options->limit = 1;

    auto resp = repository::retrieve<pmodel::Document>( retrieve );
    if ( !resp.has_value() )
    {
      LOG_WARN << "Error retrieving document " << db << ':' << M::Collection() << ':' << oid <<
        ". " << magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message;
    }
  }

  {
    auto retrieve = model::request::Retrieve<bsoncxx::document::value>{ document{} << "str" << "value modified update" << finalize  };
    retrieve.database = database;
    retrieve.collection = collection;
    retrieve.options = options::Find{};
    retrieve.options->sort = document{} << "str" << 1 << "_id" << -1 << finalize;
    retrieve.options->maxTime = std::chrono::milliseconds{ 1000 };
    retrieve.options->limit = 1000;

    auto result = repository::retrieve<pmodel::Document>( retrieve );
    if ( !resp.has_value() )
    {
      LOG_WARN << "Error retrieving documents " << db << ':' << M::Collection() << " using query. " <<
        ". " << magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message;
    }
  }
}