#include <router/router.h>

using std::operator ""s;
using std::operator ""sv;

int main()
{
  struct Request
  {
    // pass whatever you need as user data
  } request;

  const auto method = "GET"sv;
  spt::http::router::HttpRouter<const Request&, bool> r;
  r.add( "POST"sv, "/device/sensor/"sv, []( const Request&, spt::http::router::HttpRouter<const Request&, bool>::MapType args )
    {
      assert( args.empty() );
      return true;
    } );
    r.add( method, "/device/sensor/"sv, []( const Request&, auto args )
    {
      assert( args.empty() );
      return true;
    } );
    r.add( "PUT"sv, "/device/sensor/id/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/id/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/identifier/{identifier}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "identifier"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/customer/code/{code}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "code"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/facility/id/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/count/references/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/history/summary/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/history/document/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/{property}/between/{start}/{end}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 3 );
      assert( args.contains( "property"sv ) );
      assert( args.contains( "start"sv ) );
      assert( args.contains( "end"sv ) );
      return true;
    } );

  std::vector<std::string> urls =
      {
        "/device/sensor/"s,
        "/device/sensor/id/6230f3069e7c9be9ff4b78a1"s, // id=6230f3069e7c9be9ff4b78a1
        "/device/sensor/identifier/Integration Test Identifier"s, // identifier=Integration Test Identifier
        "/device/sensor/customer/code/int-test"s, // code=int-test
        "/device/sensor/history/summary/6230f3069e7c9be9ff4b78a1"s, // id=6230f3069e7c9be9ff4b78a1
        "/device/sensor/history/document/6230f3069e7c9be9ff4b78a1"s, // id=6230f3069e7c9be9ff4b78a1
        "/device/sensor/count/references/6230f3069e7c9be9ff4b78a1"s, // id=6230f3069e7c9be9ff4b78a1
        "/device/sensor/created/between/2022-03-14T20:11:50.620Z/2022-03-16T20:11:50.620Z"s, // property=created, start=2022-03-14T20:11:50.620Z, end=2022-03-16T20:11:50.620Z
      };
  for ( auto&& url : urls )
  {
    auto resp = r.route( "GET"sv, url, request );
    assert( resp );
    assert( *resp );
  }

  auto resp = r.route( "PUT"sv, "/device/sensor/"sv );
  assert( resp );
  assert( !*resp ); // PUT not configured

  resp = r.route( "POST"sv, "/device/sensor/history/document/{id}"sv );
  assert( resp );
  assert( !*resp ); // POST not configured

  try
  {
    r.add( "PUT"sv, "/device/sensor/id/{id}"sv, []( const Request&, auto args ) { return true; } );
  }
  catch ( const spt::http::router::DuplicateRouteError& e )
  {
    // Will be caught as we registered the same route earlier
    std::cerr << e.what() << '\n';
  }
}