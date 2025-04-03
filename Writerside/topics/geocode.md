# Geocode
A library of utility functions related to geo-coordinates used across a few different projects.

Provides functions for performing common activities such as:
* Calculate distance between two coordinates using Vincenty's formula.
* Check if a point falls within a bounding polygon.
* Look up the street address for a specified geo-coordinate using [positionstack](https://positionstack.com/).
* Look up the geo-coordinate for a specified street address using [positionstack](https://positionstack.com/).
* Compute the centroid of a set of geo-coordinates.
* Cluster a set of coordinates using [k-means](https://en.wikipedia.org/wiki/K-means_clustering) algorithm.
* Convert coordinates into **Open Location Code**.

A simple *shell* application (`geocodesh`) is also available for quickly invoking some of the interfaces provided
by the library.  Please note that operations involving use of the [positionstack](https://positionstack.com/) API 
needs an environment variable `POSITION_STACK_KEY` to be set with the API key.

## Examples
Examples of the library and CLI utility.

### CLI
<img src="geocodesh.png" alt="geocodesh" thumbnail="true"/>

### Library
<code-block lang="C++" collapsible="true">
<![CDATA[
#include <geocode/geocode.hpp>

int main()
{
  const auto key = std::getenv( "POSITION_STACK_KEY" );
  const auto address = spt::geocode::address( 41.9215927, -87.6953278, key );
  const auto point = spt::geocode::fromAddress( "565 5 Ave, Manhattan, New York, NY, USA", key );

  const auto points = std::array{
    spt::geocode::Point{ .latitude = 63.8066559, .longitude = -83.6791916 },
    spt::geocode::Point{ .latitude = 60.244442, .longitude = -149.6915436 },
  };
  const auto centroid = spt::geocode::centroid( std::span<const spt::geocode::Point>{ points } );

  const auto points = std::vector<spt::geocode::Point>{
    {63.8066559, -83.6791916},
    {41.9461021, -87.6977005},
    {41.9215927, -87.6953278},
    {41.9121971, -87.6807251},
    {60.244442, -149.6915436},
    {41.8827209, -87.6352386},
    {41.8839951, -87.6347198},
    {41.8830872, -87.6359787},
    {41.883255, -87.6354523},
    {41.8830147, -87.6354752},
    {41.881218, -87.6351395},
    {41.8841934, -87.6364594},
    {41.8837547, -87.6352844},
    {41.8826141, -87.6353912},
    {41.8827934, -87.6357727},
    {41.8830872, -87.6352005},
    {41.8839989, -87.632843},
    {41.8855286, -87.6347198},
    {41.8848267, -87.6368179},
    {41.943203, -87.7009201}
  };
  const auto clustered = spt::geocode::cluster( points, 32, 3 );

  const auto p1 = spt::geocode::Point{ .latitude = 51.752021, .longitude = -1.257726 };
  const auto p2 = spt::geocode::Point{ .latitude = 51.507351, .longitude = -0.127758 };
  const auto [d, a] = spt::geocode::distance( p1, p2 );

  const auto point = spt::geocode::Point{ .latitude = 63.8066559, .longitude = -83.6791916 };
  const auto str = spt::geocode::toLocationCode( point );
  const auto p = spt::geocode::fromLocationCode( str );
}
]]>
</code-block>

## Build
Build the library using [cmake](https://cmake.org):

<code-block lang="shell" collapsible="true">
git clone https://github.com/sptrakesh/geocode.git
cd geocode
cmake -DCMAKE_PREFIX_PATH="/usr/local/boost;/usr/local/cpr" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local/spt \
  -DOPENSSL_ROOT_DIR=/opt/homebrew/opt/openssl \
  -S . -B build
cmake --build build -j12
sudo cmake --install build
</code-block>
