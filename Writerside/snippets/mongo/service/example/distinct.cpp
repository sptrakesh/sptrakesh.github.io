#include <mongo-service/api/repository/repository.hpp>
#include <log/NanoLog.hpp>

namespace example
{
  struct Query
  {
    bool operator==(const Query&) const = default;
    BEGIN_VISITABLES(Query);
    VISITABLE(std::string, str);
    END_VISITABLES;
  };
}

int main()
{
  using namespace spt::mongoservice::api;
  auto distinct = model::request::Distinct{ example::Query{} };
  distinct.database = "test";
  distinct.collection = "test";
  distinct.document->filter = std::nullopt;
  distinct.document->field = "str";
  distinct.emplace();
  distinct.options->maxTime = std::chrono::milliseconds{ 1000 };

  auto result = repository::distinct( distinct );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error retrieving distinct values. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}
