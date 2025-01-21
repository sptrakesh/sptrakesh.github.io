#include <mongo-service/api/repository/repository.hpp>
#include <log/NanoLog.hpp>

int main()
{
  using namespace spt::mongoservice::api;
  auto create = model::request::CreateCollection{ options::CreateCollection{} };
  create.database = "test";
  create.collection = "test";
  create.document->timeseries.emplace();
  create.document->timeseries->timeField = "timestamp";
  create.document->timeseries->metaField = "tags";
  create.document->timeseries->granularity = options::CreateCollection::Timeseries::Granularity::hours;
  create.document->expireAfterSeconds = std::chrono::seconds{ 365*24*60*60 };

  auto result = repository::collection( create );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error creating collection. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}
