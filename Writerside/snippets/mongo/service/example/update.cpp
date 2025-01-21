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

  // merge by id
  {
    auto update = model::request::MergeForId<example::Document, example::Metadata>{};
    update.database = "test";
    update.collection = "test";
    update.document.id = bsoncxx::oid{};
    update.document.str = "value modified";
    update.metadata.emplace();
    update.metadata->project = "serialisation";
    update.metadata->product = "mongo-service";
    update.options.emplace();
    update.options->bypassValidation = true;
    update.options->upsert = true;

    auto result = repository::update( update );
    if ( !result.has_value() )
    {
      LOG_WARN << "Error updating document " << update.document.id << ". " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
    }
  }

  // replace document
  {
    auto update = model::request::Replace<example::Document, example::Metadata, model::request::IdFilter>{};
    update.database = "test";
    update.collection = "test";
    update.document.filter.emplace();
    update.document.filter->id = oid;
    update.document.replace = example::Document{};
    update.document.replace->id = oid;
    update.document.replace->str = "value replaced";
    update.metadata.emplace();
    update.metadata->project = "serialisation";
    update.metadata->product = "mongo-service";
    update.options.emplace();
    update.options->bypassValidation = true;
    update.options->upsert = true;

    auto result = repository::update( update );
    if ( !result.has_value() )
    {
      LOG_WARN << "Error updating document " << update.document.filter->id << ". " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
    }
  }

  // updating document by id filter
  {
    auto update = model::request::Update<example::Document, example::Metadata, model::request::IdFilter>{};
    update.database = "test";
    update.collection = "test";
    update.document.filter.emplace();
    update.document.filter->id = oid;
    update.document.update.emplace();
    update.document.update->str = "value modified";
    update.options.emplace();
    update.options->collation.emplace();
    update.options->collation->locale = "en";
    update.options->collation->strength = 1;

    auto result = repository::update( update );
    if ( !result.has_value() )
    {
      LOG_WARN << "Error updating document " << update.document.filter->id << ". " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
    }
  }

  // updating documents by property
  {
    auto update = model::request::Update<example::Document, example::Metadata, example::Query>{};
    update.database = "test";
    update.collection = "test";
    update.document.filter = example::Query{ .str = "value modified" };
    update.document.update = example::Document{};
    update.document.update->str = "value modified update";
    update.options = options::Update{};
    update.options->collation = options::Collation{};
    update.options->collation->locale = "en";
    update.options->collation->strength = 1;

    auto result = repository::updateMany( update );
    if ( !result.has_value() )
    {
      LOG_WARN << "Error updating document " << update.document.filter->id << ". " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
    }
  }
}