#include <mongo-service/api/repository/repository.hpp>
#include <log/NanoLog.hpp>

int main()
{
  using namespace spt::mongoservice::api;
  auto rename = model::request::RenameCollection{ "renamed" };
  rename.database = "test";
  rename.collection = "test";

  auto result = repository::collection( rename );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error renaming collection. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}
