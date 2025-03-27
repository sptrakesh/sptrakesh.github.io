//
// Created by Rakesh on 07/03/2025.
//

#pragma once

#include <chrono>
#include <format>
#include <map>
#include <source_location>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace spt::ilp
{
  struct APMRecord
  {
    using DateTime = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds>;
    using Value = std::variant<bool, int64_t, uint64_t, double, std::string>;

    struct Process
    {
      enum class Type : std::uint8_t
      {
        Function,
        Step,
        Other
      };

      explicit Process( Type type ) : type( type ) {}
      Process() = default;
      ~Process() = default;
      Process( Process&& ) = default;
      Process& operator=( Process&& ) = default;

      Process( const Process& ) = delete;
      Process& operator=( const Process& ) = delete;

      std::map<std::string, std::string, std::less<>> tags;
      std::map<std::string, Value, std::less<>> values;
      DateTime timestamp{ std::chrono::high_resolution_clock::now() };
      std::chrono::nanoseconds duration{ 0 };
      Type type{ Type::Other };
    };

    explicit APMRecord( std::string_view id ) : id{ id } {}
    ~APMRecord() = default;
    APMRecord( APMRecord&& ) = default;
    APMRecord& operator=( APMRecord&& ) = default;

    APMRecord( const APMRecord& ) = delete;
    APMRecord& operator=( const APMRecord& ) = delete;

    std::vector<Process> processes;
    std::map<std::string, std::string, std::less<>> tags;
    std::map<std::string, Value, std::less<>> values;
    std::string id;
    std::string application;
    DateTime timestamp{ std::chrono::high_resolution_clock::now() };
    std::chrono::nanoseconds duration{ 0 };
  };

  template <typename T>
  concept Record = requires( T t )
  {
    std::is_same_v<decltype(t.timestamp), std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds>>;
    std::is_same_v<decltype(t.duration), std::chrono::nanoseconds>;
  };

  APMRecord createAPMRecord( std::string_view id, std::string_view application, APMRecord::Process::Type type,
    std::size_t size, std::source_location loc = std::source_location::current() );

  APMRecord::Process& addProcess( APMRecord& apm, APMRecord::Process::Type type, std::source_location loc = std::source_location::current() );

  APMRecord::Process& addException( APMRecord& apm, const std::exception& ex, std::string_view prefix,
    std::source_location loc = std::source_location::current() );

  template <Record T>
  void setDuration( T& record )
  {
    record.duration = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now() - record.timestamp );
  }

  template <Record T>
  void addCurrentFunction( T& record, const std::source_location loc = std::source_location::current(), std::string_view tagName = "caller" )
  {
    record.values.try_emplace( std::format( "{}_file", tagName ), loc.file_name() );
    record.values.try_emplace( std::format( "{}_line", tagName ), static_cast<uint64_t>( loc.line() ) );
    if ( auto fn = std::string{ loc.function_name() }; !fn.empty() ) record.values.try_emplace( std::format( "{}_function", tagName ), std::move( fn ) );
  }
}