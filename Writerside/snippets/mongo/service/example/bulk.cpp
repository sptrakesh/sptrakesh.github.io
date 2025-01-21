#include <mongo-service/api/repository/repository.hpp>
#include <log/NanoLog.hpp>

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

  struct Metadata
  {
    explicit Metadata( bsoncxx::document::view bson ) { spt::util::unmarshall( *this, bson ); }
    Metadata() = default;
    ~Metadata() = default;
    Metadata(Metadata&&) = default;
    Metadata& operator=(Metadata&&) = default;
    bool operator==(const Metadata&) const = default;

    Metadata(const Metadata&) = delete;
    Metadata& operator=(const Metadata&) = delete;

    BEGIN_VISITABLES(Metadata);
    VISITABLE(std::string, project);
    VISITABLE(std::string, product);
    END_VISITABLES;
  };
}

int main()
{
  using namespace spt::mongoservice::api;
  auto bulk = model::request::Bulk<example::Document, example::Metadata, model::request::IdFilter>{};
  for ( auto i = 0; i < 10; ++i )
  {
    bulk.document.insert.emplace_back();
    bulk.document.remove.emplace_back();
    bulk.document.remove.back().id = bulk.document.insert.back().id;
  }
  bulk.database = "test";
  bulk.collection = "test";

  auto result = repository::bulk( bulk );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error executing bulk statements. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}
