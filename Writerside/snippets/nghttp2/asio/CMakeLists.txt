cmake_minimum_required(VERSION 3.31)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
  list(APPEND CMAKE_PREFIX_PATH "/usr/local/boost")
  list(APPEND CMAKE_PREFIX_PATH "/usr/local/spt")
  list(APPEND CMAKE_PREFIX_PATH "/usr/local/mongo")
  list(APPEND CMAKE_PREFIX_PATH "/usr/local/nghttp2")
  include_directories(
    /usr/local/boost/include
    /usr/local/mongo/include
    /usr/local/nghttp2/include
    /usr/local/spt/include
    /opt/homebrew/opt/libnghttp2/include
  )
  link_directories(
    /usr/local/nghttp2/lib
    /opt/homebrew/opt/libnghttp2/lib
  )
  find_package(bsoncxx REQUIRED)
else()
  list(APPEND CMAKE_PREFIX_PATH "/opt/spt")
  list(APPEND CMAKE_PREFIX_PATH "/opt/local")
  include_directories(/opt/local/include /opt/spt/include)
  link_directories(/opt/local/lib /opt/spt/lib)
endif (APPLE)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.86.0 REQUIRED json)
find_package(OpenSSL 1.0.1)
find_package(NanoLog REQUIRED COMPONENTS nanolog)
find_package(Nghttp2Asio REQUIRED)
find_package(Http2Framework)

include_directories(
  ${OPENSSL_INCLUDE_DIR}
)

add_executable(server server.cpp)

target_link_libraries(server PRIVATE Boost::json http2_framework nghttp2_asio nghttp2 nanolog::nanolog ${OPENSSL_LIBRARIES})
