# Catch2 HTML Reporter
<show-structure for="chapter,tab" depth="2"/>

A simple [Reporter](https://catch2-temp.readthedocs.io/en/latest/reporters.html) 
implementation that generates **HTML** reports for test suites written
using [Catch2](https://github.com/catchorg/Catch2).

## Output Files
The reporter produces the following output files:
* `test-results/index.html` - A simple index file that lists the individual test results
  files, and the *summary* and *results* files.
* `test-results/summary.html` - Test result summary in tabular format.
* `test-results/results.html` - The full aggregated results of the test suite.
* `test-results/<TestName>.html` - Report with results from executing each test case.

## Implementation
The reporter extends the [Catch::StreamingReporterBase](https://catch2-temp.readthedocs.io/en/latest/developer_api/index.html#_CPPv4N5Catch21StreamingReporterBaseE) class.
I chose to extend the `Catch::StreamingReporterBase` class instead of `Catch::CumulativeReporterBase` to
also produce console output when the test suite is run.  The actual HTML report generation
is done only after the test suite has completed.

### testRunStarting
```C++
    void testRunStarting( const Catch::TestRunInfo& info ) override
    {
      StreamingReporterBase::testRunStarting( info );
      suites.reserve( 16 );
      seed = Catch::getSeed();
      start = std::chrono::high_resolution_clock::now();
    }
```

Populate the `seed` for the run (we do not use it at present), as well as start the clock
used to measure execution time.

### testCaseStarting
```C++
    void testCaseStarting( const Catch::TestCaseInfo& info ) override
    {
      StreamingReporterBase::testCaseStarting( info );
      currentSection = nullptr;
      suites.push_back( phr::create<phr::Suite>( info.name ) );
      std::println( "\033[1;34m{}\033[0m", info.name );
    }
```

Add the suite to the vector of suites for this run.  Print the test case name to the console.

### sectionStarting
```C++
void sectionStarting( const Catch::SectionInfo& info )
```

Add the section to the suite if not already added.  Keep a `pointer` to the section.
This is used to track the nested hierarchy of sections in a test case.

### assertionEnded
```C++
    void assertionEnded( const Catch::AssertionStats& stats ) override
    {
      if ( !stats.assertionResult.isOk() )
      {
        std::println( "\033[1;31mAssertion '{}' failed: {} at {}:{}\033[0m",
          stats.assertionResult.getExpression(), stats.assertionResult.getMessage().data(),
          stats.assertionResult.getSourceInfo().file, stats.assertionResult.getSourceInfo().line );
      }
    }
```

This over-ride is used to print assertion [failures](#catch2-html-reporter-example-failure) to the console.
This level of detail is not appropriate in the HTML reports, and is
not added to the output files.

### sectionEnded
```C++
void sectionEnded( const Catch::SectionStats& stats )
```

Update the assertion statistics for the tracked section.  Output the assertion statistics
to the console.

### testCaseEnded
```C++
void testCaseEnded( const Catch::TestCaseStats& stats )
```

Accumulate the assertion statistics for the section to the suite.  Print out the assertion
statistics to the console.

### testRunEnded
```C++
void testRunEnded( const Catch::TestRunStats& stats )
```

Output the test run statistics to the console.  Generate the HTML report.

### Code
You can view the full source code for the [reporter](https://gist.github.com/sptrakesh/48b2e66a208ccf83b276063885b3e29b).

<tabs id="catch2-html-reporter">
  <tab title="Section" id="catch2-html-reporter-section">
    A simple structure used to represent a test suite section.
    <code-block lang="C++" collapsible="false">
    <![CDATA[
    struct Section
    {
      Catch::Counts assertions;
      std::vector<Section> sections;
      Section* parent{ nullptr };
      std::string name;
      double duration{ 0.0 };
      bool printed{ false };
    };
    ]]>
    </code-block>
  </tab>
  <tab title="Suite" id="catch2-html-reporter-suite">
    A simple structure used to represent a test suite.
    <code-block lang="C++" collapsible="false">
    <![CDATA[
    struct Suite
    {
      Catch::Counts assertions;
      std::vector<Section> sections;
      std::string name;
      decltype(std::chrono::high_resolution_clock::now()) start{ std::chrono::high_resolution_clock::now() };
      decltype(std::chrono::high_resolution_clock::now()) end;

      [[nodiscard]] std::string filename() const
      {
        return std::format( "{}.html", boost::algorithm::replace_all_copy( name, " ", "-" ) );
      }

      [[nodiscard]] std::chrono::nanoseconds duration() const
      {
        return end - start;
      }
    };
    ]]>
    </code-block>
  </tab>
  <tab title="TestRunStarting" id="catch2-html-reporter-runstarting">
    Over-ridden implementation of the <code>testRunStarting</code> method.
    <code-block lang="C++" collapsible="false">
    <![CDATA[
    void testRunStarting( const Catch::TestRunInfo& info ) override
    {
      StreamingReporterBase::testRunStarting( info );
      suites.reserve( 16 );
      seed = Catch::getSeed();
      start = std::chrono::high_resolution_clock::now();
    }
    ]]>
    </code-block>
  </tab>
  <tab title="TestCaseStarting" id="catch2-html-reporter-casestarting">
    Over-ridden implementation of the <code>testCaseStarting</code> method.
    <code-block lang="C++" collapsible="false">
    <![CDATA[
    void testCaseStarting( const Catch::TestCaseInfo& info ) override
    {
      StreamingReporterBase::testCaseStarting( info );
      currentSection = nullptr;
      suites.push_back( phr::create<phr::Suite>( info.name ) );
      std::println( "\033[1;34m{}\033[0m", info.name );
    }
    ]]>
    </code-block>
  </tab>
  <tab title="SectionStarting" id="catch2-html-reporter-sectionstarting">
    Over-ridden implementation of the <code>sectionStarting</code> method.
    <code-block lang="C++" collapsible="false">
    <![CDATA[
    void sectionStarting( const Catch::SectionInfo& info ) override
    {
      StreamingReporterBase::sectionStarting( info );
      auto test = phr::create<phr::Section>( info.name );
      test.parent = currentSection;

      if ( test.name == suites.back().name ) return;

      const auto add = [this]( std::vector<phr::Section>& sections, phr::Section&& sec )
      {
        if ( auto iter = ranges::find_if( sections, [&sec]( const auto& tc ) { return tc.name == sec.name; } );
          iter == ranges::end( sections ) )
        {
          sections.push_back( std::move( sec ) );
          currentSection = &sections.back();
        }
        else
        {
          currentSection = iter.base();
        }
      };

      add( currentSection ? currentSection->sections : suites.back().sections, std::move( test ) );
    }
    ]]>
    </code-block>
  </tab>
  <tab title="SectionEnded" id="catch2-html-reporter-sectionended">
    Over-ridden implementation of the <code>sectionEnded</code> method.
    <code-block lang="C++" collapsible="false">
    <![CDATA[
    void sectionEnded( const Catch::SectionStats& stats ) override
    {
      StreamingReporterBase::sectionEnded( stats );

      if ( !currentSection ) return;
      if ( const auto sec = phr::create<phr::Section>( stats.sectionInfo.name ); sec.name == suites.back().name ) return;

      currentSection->assertions.passed += stats.assertions.passed;
      currentSection->assertions.failed += stats.assertions.failed;
      currentSection->assertions.failedButOk += stats.assertions.failedButOk;
      currentSection->assertions.skipped += stats.assertions.skipped;
      currentSection->duration += stats.durationInSeconds;

      const auto indent = []( const phr::Section& sec ) -> std::string
      {
        auto indent = std::string{};
        indent.reserve( 16 );
        indent.append( 2, ' ' );

        auto root = sec.parent;
        while ( root )
        {
          indent.append( 2, ' ' );
          root = root->parent;
        }

        return indent;
      };

      const auto style = []( const phr::Section& sec ) -> std::string
      {
        if ( sec.assertions.allPassed() ) return "\033[36m";
        return "\033[33m";
      };

      const auto symbol = [] ( const phr::Section& sec ) -> std::string
      {
        return sec.assertions.allPassed() ? "✅" : "❌";
      };

      auto messages = std::vector<std::string>{};
      messages.reserve( 8 );
      if ( !currentSection->printed )
      {
        messages.push_back( std::format( "{}{}{} {}\033[0m", indent( *currentSection ), style( *currentSection ), symbol( *currentSection ), currentSection->name ) );
        currentSection->printed = true;
      }

      auto root = currentSection->parent;
      while ( root && !root->printed )
      {
        messages.push_back( std::format( "{}{}{} {}", indent( *root ), style( *root ), symbol( *root ), root->name ) );
        root->printed = true;
        root = root->parent;
      }

      for ( const auto& msg : messages | ranges::views::reverse ) std::println( "{}", msg );

      currentSection = currentSection->parent;
    }
    ]]>
    </code-block>
  </tab>
  <tab title="TestCaseEnded" id="catch2-html-reporter-testcaseended">
    Over-ridden implementation of the <code>testCaseEnded</code> method.
    <code-block lang="C++" collapsible="false">
    <![CDATA[
    void testCaseEnded( const Catch::TestCaseStats& stats ) override
    {
      currentSection = nullptr;
      if ( !stats.testInfo ) return;

      auto& suite = suites.back();
      suite.assertions.passed += stats.totals.assertions.passed;
      suite.assertions.failed += stats.totals.assertions.failed;
      suite.assertions.failedButOk += stats.totals.assertions.failedButOk;
      suite.assertions.skipped += stats.totals.assertions.skipped;
      suite.end = std::chrono::high_resolution_clock::now();

      std::println( "\033[1;34m{} \033[0m\033[1m({} seconds)\033[0m", suite.name, std::chrono::duration_cast<std::chrono::duration<double>>( suite.duration() ).count() );
      std::println( "\033[1;34m  Assertions - \033[0m\033[1;32mPassed: {}; \033[0m\033[1;31mFailed: {}, \033[0m\033[1;33mFailedOk: {}, \033[0m\033[1;35mSkipped: {}, \033[0m\033[1mTotal: {}\033[0m",
        suite.assertions.passed, suite.assertions.failed, suite.assertions.failedButOk,
        suite.assertions.skipped, suite.assertions.total() );
    }
    ]]>
    </code-block>
  </tab>
  <tab title="TestRunEnded" id="catch2-html-reporter-testrunended">
    Over-ridden implementation of the <code>testRunEnded</code> method.
    <code-block lang="C++" collapsible="false">
    <![CDATA[
    void testRunEnded( const Catch::TestRunStats& stats ) override
    {
      StreamingReporterBase::testRunEnded( stats );

      std::println( "\033[1;34m{}\033[0m", stats.runInfo.name.data() );
      std::println( "\033[1;34m  Test Cases - \033[0m\033[1;32mPassed: {}; \033[0m\033[1;31mFailed: {}, \033[0m\033[1;33mFailedOk: {}, \033[0m\033[1;35mSkipped: {}, \033[0m\033[1mTotal: {}\033[0m",
        stats.totals.testCases.passed, stats.totals.testCases.failed, stats.totals.testCases.failedButOk,
        stats.totals.testCases.skipped, stats.totals.testCases.total() );
      std::println( "\033[1;34m  Assertions -  \033[0m\033[1;32mPassed: {}; \033[0m\033[1;31mFailed: {}, \033[0m\033[1;33mFailedOk: {}, \033[0m\033[1;35mSkipped: {}; \033[0m\033[1mTotal: {}\033[0m",
        stats.totals.assertions.passed, stats.totals.assertions.failed, stats.totals.assertions.failedButOk,
        stats.totals.assertions.skipped, stats.totals.assertions.total() );

      std::println( "\033[1;34m  Duration - \033[0m\033[1m{} \033[0m\033[1;34mseconds\033[0m", std::chrono::duration_cast<std::chrono::duration<double>>( std::chrono::high_resolution_clock::now() - start ).count() );
      phr::generate( suites, "test-results" );
    }
    ]]>
    </code-block>
  </tab>
</tabs>

## Example Output
The following screen captures show the output HTML files generated by the reporter
for one of my test suites.

<tabs id="catch2-html-reporter-examples">
  <tab title="Console Output Start" id="catch2-html-reporter-example-start">
    <img src="console-run-begin.png" alt="Console output begin" thumbnail="true"/>
  </tab>
  <tab title="Console Output Failure" id="catch2-html-reporter-example-failure">
    <img src="console-run-failure.png" alt="Console output failure" thumbnail="true"/>
  </tab>
  <tab title="Console Output End" id="catch2-html-reporter-example-end">
    <img src="console-run-end.png" alt="Console output begin" thumbnail="true"/>
  </tab>
  <tab title="Index" id="catch2-html-reporter-example-index">
    <img src="catch-reporter-index.png" alt="Index file" thumbnail="true"/>
  </tab>
  <tab title="Summary" id="catch2-html-reporter-example-summary">
    <img src="catch-reporter-summary.png" alt="Test Summary file" thumbnail="true"/>
  </tab>
  <tab title="Test Case" id="catch2-html-reporter-example-testcase">
    <img src="catch-reporter-testcase.png" alt="Test Case file" thumbnail="true"/>
  </tab>
  <tab title="Aggregated Results" id="catch2-html-reporter-example-aggregate">
    <img src="catch-reporter-aggregate.png" alt="Test Case file" thumbnail="true"/>
  </tab>
</tabs>