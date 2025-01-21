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
  using bsoncxx::builder::stream::open_array;
  using bsoncxx::builder::stream::close_array;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;

  auto builder = model::request::TransactionBuilder{ "unit", "test" };

  auto insert = model::request::Create<pmodel::Document, pmodel::Metadata>{};
  insert.database = "test";
  insert.collection = "test";
  insert.application = "unitTest";
  insert.document.str = "value";
  insert.metadata = pmodel::Metadata{};
  insert.metadata->project = "serialisation";
  insert.metadata->product = "mongo-service";
  insert.options = options::Insert{};
  insert.options->writeConcern = options::WriteConcern{};
  insert.options->writeConcern->tag = "test";
  insert.options->writeConcern->majority = std::chrono::milliseconds{ 100 };
  insert.options->writeConcern->timeout = std::chrono::milliseconds{ 250 };
  insert.options->writeConcern->nodes = 2;
  insert.options->writeConcern->acknowledgeLevel = options::WriteConcern::Level::Majority;
  insert.options->writeConcern->journal = true;
  insert.options->bypassValidation = true;
  insert.options->ordered = true;
  builder.addCreate( insert );

  auto update = model::request::Update<bsoncxx::document::value, pmodel::Metadata, model::request::IdFilter>{};
  update.document.filter = model::request::IdFilter{};
  update.document.filter->id = bsoncxx::oid{};
  update.document.update = document{} <<
    "$unset" << open_document << "obsoleteProperty" << 1 << close_document <<
    "$set" <<
      open_document <<
        "modified" << bsoncxx::types::b_date{ std::chrono::system_clock::now() } <<
        "user._id" << bsoncxx::oid{} <<
      close_document <<
    finalize;
  update.database = "unit";
  update.collection = "test";
  update.application = "unitTest";
  update.options = options::Update{};
  update.options->collation = options::Collation{};
  update.options->collation->locale = "en";
  update.options->collation->strength = 1;
  update.options->writeConcern = options::WriteConcern{};
  update.options->writeConcern->tag = "test";
  update.options->writeConcern->majority = std::chrono::milliseconds{ 100 };
  update.options->writeConcern->timeout = std::chrono::milliseconds{ 250 };
  update.options->writeConcern->nodes = 2;
  update.options->writeConcern->acknowledgeLevel = options::WriteConcern::Level::Majority;
  update.options->writeConcern->journal = true;
  update.options->bypassValidation = false;
  update.options->upsert = false;
  builder.addUpdate( update );

  auto remove = model::request::Delete<bsoncxx::document::value, pmodel::Metadata>{ document{} << finalize  };
  remove.database = "unit";
  remove.collection = "test";
  remove.application = "unitTest";
  remove.options = options::Delete{};
  remove.options->collation = options::Collation{};
  remove.options->collation->locale = "en";
  remove.options->collation->strength = 1;
  remove.options->hint = document{} << "name" << 1 << finalize;
  remove.options->writeConcern = options::WriteConcern{};
  remove.options->writeConcern->tag = "test";
  remove.options->writeConcern->majority = std::chrono::milliseconds{ 100 };
  remove.options->writeConcern->timeout = std::chrono::milliseconds{ 250 };
  remove.options->writeConcern->nodes = 2;
  remove.options->writeConcern->acknowledgeLevel = options::WriteConcern::Level::Majority;
  remove.options->writeConcern->journal = true;
  remove.options->let = document{} <<
    "vars" <<
      open_document <<
        "total" <<
          open_document <<
            "$add" << open_array << "$price" << "$tax" << close_array <<
          close_document <<
        "discounted" <<
          open_document <<
            "$cond" <<
              open_document <<
                "if" << "$applyDiscount" << "then" << 0.9 << "else" << 1 <<
              close_document <<
          close_document <<
      close_document <<
    "in" <<
      open_document <<
        "$multiply" << open_array << "$$total" << "$$discounted" << close_array <<
      close_document <<
    finalize;
  builder.addRemove( remove );

  const auto bson = builder.build();
  auto result = repository::transaction( bson );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error executing transaction. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}
