//
// Created by Rakesh on 6/4/21.
//

#include <charconv>
#include <bsoncxx/builder/stream/document.hpp>
#include <http2/framework/common.hpp>
#include <ilp/apmrecord.hpp>
#include <log/NanoLog.hpp>
#include <magic_enum/magic_enum.hpp>

namespace spt::ilp::bson
{
  template<class... Ts> struct overload : Ts... { using Ts::operator()...; };

  void serialise( const std::map<std::string, spt::ilp::APMRecord::Value, std::less<>>& map,
    bsoncxx::builder::stream::document& builder, std::string_view property )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;

    auto values = document{};
    for ( const auto& pair : map )
    {
      std::visit( overload
        {
          [&values, &pair]( bool v ){ values << pair.first << v; },
          [&values, &pair]( int64_t v ){ values << pair.first << v; },
          [&values, &pair]( uint64_t v ){ values << pair.first << static_cast<int64_t>( v ); },
          [&values, &pair]( double v ){ values << pair.first << v; },
          [&values, &pair]( const std::string& v ){ values << pair.first << v; },
        }, pair.second );
    }
    builder << property << (values << finalize);
  }

  bsoncxx::document::value serialise( const spt::ilp::APMRecord::Process& process )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;

    auto doc = document{};
    doc << "type" << magic_enum::enum_name( process.type ) <<
      "timestamp" << bsoncxx::types::b_int64{ std::chrono::duration_cast<std::chrono::nanoseconds>( process.timestamp.time_since_epoch() ).count() } <<
      "duration" << bsoncxx::types::b_int64{ process.duration.count() };

    auto tags = document{};
    for ( const auto& [key, value] : process.tags ) tags << key << value;
    doc << "tags" << (tags << finalize);

    serialise( process.values, doc, "values" );

    return doc << finalize;
  }

  bsoncxx::document::value serialise( const spt::ilp::APMRecord& apm )
  {
    using bsoncxx::builder::stream::array;
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;

    auto doc = document{};
    doc << "_id" << bsoncxx::oid{ apm.id } <<
      "application" << apm.application <<
      "timestamp" << bsoncxx::types::b_int64{ std::chrono::duration_cast<std::chrono::nanoseconds>( apm.timestamp.time_since_epoch() ).count() } <<
      "duration" << bsoncxx::types::b_int64{ apm.duration.count() };

    auto tags = document{};
    for ( const auto& [key, value] : apm.tags ) tags << key << value;
    doc << "tags" << (tags << finalize);

    serialise( apm.values, doc, "values" );

    auto arr = array{};
    for ( const auto& proc : apm.processes ) arr << serialise( proc );
    doc << "processes" << (arr << finalize);

    return doc << finalize;
  }

  template <typename Record>
  void parse( Record& r, bsoncxx::document::view bson )
  {
    using spt::util::bsonValueIfExists;

    if ( auto tags = bsonValueIfExists<bsoncxx::document::view>( "tags", bson ); tags )
    {
      for ( const auto& elem : *tags )
      {
        r.tags.try_emplace( std::string{ elem.key() }, std::string{ elem.get_string().value } );
      }
    }

    if ( auto values = bsonValueIfExists<bsoncxx::document::view>( "values", bson ); values )
    {
      for ( const auto& elem : *values )
      {
        switch ( elem.type() )
        {
          using enum bsoncxx::type;
        case k_bool:
          r.values.try_emplace( std::string{ elem.key() }, elem.get_bool().value );
          break;
        case k_int64:
          r.values.try_emplace( std::string{ elem.key() }, elem.get_int64().value );
          break;
        case k_double:
          r.values.try_emplace( std::string{ elem.key() }, elem.get_double().value );
          break;
        case k_string:
          r.values.try_emplace( std::string{ elem.key() }, std::string{ elem.get_string().value } );
          break;
        default:
          LOG_CRIT << "Invalid type " << bsoncxx::to_string( elem.type() ) << " for key " << elem.key() << " in apm record";
          break;
        }
      }
    }

  }

  spt::ilp::APMRecord::Process parseProcess( bsoncxx::document::view bson )
  {
    using spt::util::bsonValue;
    using spt::util::bsonValueIfExists;
    auto p = spt::ilp::APMRecord::Process();

    if ( auto t = magic_enum::enum_cast<spt::ilp::APMRecord::Process::Type>( bsonValue<std::string>( "type", bson ) ); t ) p.type = *t;
    if ( auto v = bsonValueIfExists<std::chrono::nanoseconds>( "timestamp", bson ); v ) p.timestamp = spt::ilp::APMRecord::DateTime{ *v };
    if ( auto v = bsonValueIfExists<std::chrono::nanoseconds>( "duration", bson ); v ) p.duration = *v;
    parse( p, bson );
    return p;
  }

  spt::ilp::APMRecord parse( bsoncxx::document::view bson )
  {
    using spt::util::bsonValue;
    using spt::util::bsonValueIfExists;

    auto apm = spt::ilp::APMRecord{ bsonValue<bsoncxx::oid>( "_id", bson ).to_string() };
    if ( auto str = bsonValueIfExists<std::string>( "application", bson ); str ) apm.application = *str;
    if ( auto v = bsonValueIfExists<std::chrono::nanoseconds>( "timestamp", bson ); v ) apm.timestamp = spt::ilp::APMRecord::DateTime{ *v };
    if ( auto v = bsonValueIfExists<std::chrono::nanoseconds>( "duration", bson ); v ) apm.duration = *v;
    parse( apm, bson );

    if ( auto procs = bsonValueIfExists<bsoncxx::array::view>( "processes", bson ); procs )
    {
      apm.processes.reserve( std::distance( procs->begin(), procs->end() ) );
      for ( const auto& item : *procs )
      {
        apm.processes.emplace_back( parseProcess( item.get_document().value ) );
      }
    }

    return apm;
  }
}