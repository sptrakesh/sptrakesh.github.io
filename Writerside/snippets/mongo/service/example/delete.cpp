#include <mongo-service/api/repository/repository.hpp>
#include <log/NanoLog.hpp>

namespace example
{
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
  auto remove = model::request::Delete<model::request::IdFilter, example::Metadata>{ model::request::IdFilter{ oid } };
  remove.database = "test";
  remove.collection = "test";

  auto result = repository::remove( remove );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error deleting document. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}
