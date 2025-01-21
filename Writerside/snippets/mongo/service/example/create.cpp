#include <mongo-service/api/repository/repository.hpp>

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
  auto insert = model::request::Create<example::Document, example::Metadata>{};
  insert.database = database;
  insert.collection = collection;
  insert.document.str = "value";
  insert.metadata.emplace();
  insert.metadata->project = "serialisation";
  insert.metadata->product = "mongo-service";
  insert.options.emplace();
  insert.options->bypassValidation = true;
  insert.options->ordered = true;

  auto result = repository::create( insert );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error creating document. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}