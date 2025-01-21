#include <mongo-service/api/repository/repository.hpp>
#include <log/NanoLog.hpp>

int main()
{
  using namespace spt::mongoservice::api;
  auto index = model::request::DropIndex<bsoncxx::document::value>{ document{} << "str" << -1 << finalize };
  index.database = "test";
  index.collection = "test";
  index.options = options::Index{};
  index.options->collation = options::Collation{};
  index.options->collation->locale = "en";
  index.options->collation->strength = 1;
  index.options->name = "statusidx";
  index.options->defaultLanguage = "en";
  index.options->languageOverride = "en-gb";
  index.options->expireAfterSeconds = std::chrono::seconds{ 1000 };
  index.options->background = true;
  index.options->unique = false;
  index.options->hidden = false;
  index.options->sparse = false;

  auto result = repository::dropIndex( index );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error dropping index. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}
