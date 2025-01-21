#include <mongo-service/api/repository/repository.hpp>

namespace example
{
  struct Document
  {
    struct Tags
    {
      Tags() = default;
      ~Tags() = default;
      Tags(Tags&&) = default;
      Tags& operator=(Tags&&) = default;
      bool operator==(const Tags&) const = default;

      Tags(const Tags&) = delete;
      Tags& operator=(const Tags&) = delete;

      BEGIN_VISITABLES(Tags);
      VISITABLE(std::string, str);
      VISITABLE_DIRECT_INIT(int64_t, integer, {5});
      VISITABLE_DIRECT_INIT(double, floating, {10.345});
      VISITABLE_DIRECT_INIT(bool, boolean, {true});
      END_VISITABLES;
    };

    Document() = default;
    ~Document() = default;
    Document(Document&&) = default;
    Document& operator=(Document&&) = default;
    bool operator==(const Document&) const = default;

    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    BEGIN_VISITABLES(Document);
    VISITABLE(Tags, tags);
    VISITABLE(bsoncxx::oid, id);
    VISITABLE_DIRECT_INIT(spt::util::DateTimeMs, timestamp, {std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() )});
    END_VISITABLES;
  };
}

int main()
{
  using namespace spt::mongoservice::api;
  auto insert = model::request::CreateTimeseries<example::Document>{};
  insert.database = "test";
  insert.collection = "ts-test";
  insert.document.tags.str = "value";

  auto result = repository::create( insert );
  if ( !result.has_value() )
  {
    LOG_WARN << "Error creating timeseries document. " << magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
  }
}
