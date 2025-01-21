#include <mongo-service/api/repository/repository.hpp>
#include <log/NanoLog.hpp>

int main()
{
  using namespace spt::mongoservice::api;
  auto index = model::request::Index<bsoncxx::document::value>{ document{} << "str" << -1 << finalize };
  index.database = "test";
  index.collection = "test";
  index.options.emplace();
  index.options->collation.emplace();
  index.options->collation->locale = "en";
  index.options->collation->strength = 1;
  index.options->name = "statusidx";
  index.options->defaultLanguage = "en";
  index.options->languageOverride = "en-gb";
  index.options->expireAfterSeconds = std::chrono::duration_cast<std::chrono::seconds>( std::chrono::days{ 365 } );
  index.options->background = true;
  index.options->unique = false;
  index.options->hidden = false;
  index.options->sparse = true;

  auto result = repository::index( index );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error ensuring index. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}
