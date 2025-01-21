#include <mongo-service/api/repository/repository.hpp>
#include <log/NanoLog.hpp>

int main()
{
  using namespace spt::mongoservice::api;
  auto remove = model::request::DropCollection{};
  remove.document.clearVersionHistory = true;
  remove.database = "test";
  remove.collection = "test";

  auto result = repository::dropCollection( remove );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error dropping collection. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}
