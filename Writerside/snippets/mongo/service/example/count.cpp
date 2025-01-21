#include <mongo-service/api/repository/repository.hpp>
#include <log/NanoLog.hpp>

int main()
{
  using namespace spt::mongoservice::api;
  auto count = model::request::Count<bsoncxx::document::value>{ document{} << finalize  };
  count.database = "test";
  count.collection = "test";
  count.options.emplace();
  count.options->maxTime = std::chrono::milliseconds{ 500 };
  count.options->limit = 100'000;

  auto result = repository::count( count );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error counting documents. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}