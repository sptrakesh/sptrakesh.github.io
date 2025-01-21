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
}

int main()
{
  using namespace spt::mongoservice::api;
  using bsoncxx::builder::stream::array;
  using bsoncxx::builder::stream::open_array;
  using bsoncxx::builder::stream::close_array;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;

  auto pipeline = model::request::Pipeline{};
  pipeline.database = "test";
  pipeline.collection = "test";
  pipeline.addStage( "$match", document{} << "str" << "value modified update" << finalize );
  pipeline.addStage( "$sort", document{} << "_id" << -1 << finalize );
  pipeline.addStage( "$limit", 1 );
  pipeline.options = options::Find{};
  pipeline.options->maxTime = std::chrono::milliseconds{ 500 };
  pipeline.options->limit = 10000;

  auto result = repository::pipeline<example::Document>( pipeline );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error retrieving documents using pipeline. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}
