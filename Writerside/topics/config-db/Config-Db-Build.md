# Build

Standard cmake build procedure. See [Dockerfile](https://github.com/sptrakesh/config-db/blob/master/docker/Dockerfile)
for build sequence. Ensure the dependencies are available under the following paths:

* **MacOSX** - Various dependencies installed under the `/usr/local/<dependency>` path. See 
  dependencies for scripts used to install the dependencies.
* **UNIX** - All dependencies installed under the `/opt/local` path.
* **Windows** - Most dependencies installed under the `\opt\local` path. A few dependencies 
  also installed via `vcpkg` under `\opt\src\vcpkg`.

## UNIX
Check out the project and build. Install dependencies. These instructions are for setting 
up on **Mac OS X**. For **Linux**, the assumption is that all dependencies are installed 
under `/opt/local`.

<tabs id="config-db-macosx">
  <tab title="Boost" id="build-boost-macosx">
    <include from="boost.topic" element-id="boost-macosx"/>
  </tab>
  <tab title="RocksDb" id="build-rocksdb-macosx">
    <a href="https://rocksdb.org/">RocksDB</a> installed using the following script:
    <code-block lang="SHELL" collapsible="true">
<![CDATA[
#!/bin/sh

PREFIX=/usr/local/rocksdb
VERSION=6.27.3

cd /tmp
if [ -d rocksdb ]
then
rm -rf rocksdb gflags
fi

if [ -d $PREFIX ]
then
sudo rm -rf $PREFIX
fi

(git clone https://github.com/gflags/gflags.git \
&& cd gflags \
&& mkdir build_ && cd build_ \
&& cmake \
-DBUILD_SHARED_LIBS=OFF \
-DBUILD_STATIC_LIBS=ON \
-DBUILD_TESTING=OFF \
-DBUILD_gflags_LIBS=ON \
-DINSTALL_HEADERS=ON \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_PREFIX_PATH=$PREFIX \
-DCMAKE_INSTALL_PREFIX=$PREFIX \
-DCMAKE_INSTALL_LIBDIR=lib \
.. \
&& make -j12 \
&& sudo make install)

(git clone -b v${VERSION} https://github.com/facebook/rocksdb.git \
&& cd rocksdb \
&& mkdir build && cd build \
&& cmake -DWITH_TESTS=OFF \
-DWITH_ALL_TESTS=OFF \
-DCMAKE_CXX_STANDARD=17 \
-DCMAKE_BUILD_TYPE=Release \
-DROCKSDB_BUILD_SHARED=OFF \
-DCMAKE_PREFIX_PATH=$PREFIX \
-DCMAKE_INSTALL_PREFIX=$PREFIX \
-DCMAKE_INSTALL_LIBDIR=lib \
.. \
&& make -j12 \
&& sudo make install)
]]>
    </code-block>
  </tab>
  <tab title="flatbuffers" id="build-flatbuffers-macosx">
    <a href="https://google.github.io/flatbuffers">flatbuffers</a> installed using the following script:
    <code-block lang="SHELL" collapsible="true">
<![CDATA[
#!/bin/sh

NAME=flatbuffers
PREFIX=/usr/local/flatbuffers
VERSION=2.0.0

cd /tmp
if [ -d $NAME ]
then
rm -rf $NAME
fi

if [ -d $PREFIX ]
then
sudo rm -rf $PREFIX
fi

git clone -b v${VERSION} https://github.com/google/flatbuffers.git \
&& cd $NAME \
&& mkdir build && cd build \
&& cmake -DFLATBUFFERS_BUILD_TESTS=OFF \
-DFLATBUFFERS_BUILD_CPP17=ON \
-DFLATBUFFERS_ENABLE_PCH=ON \
-DCMAKE_PREFIX_PATH=$PREFIX \
-DCMAKE_INSTALL_PREFIX=$PREFIX \
-DCMAKE_INSTALL_LIBDIR=lib \
.. \
&& make -j12 \
&& sudo make install
]]>
    </code-block>
  </tab>
  <tab title="nghttp2" id="build-nghttp2-macosx">
    <include from="nghttp2-asio.md" element-id="nghttp2::macosx::lib"></include>
    <include from="nghttp2-asio.md" element-id="nghttp2::macosx::project"></include>
  </tab>
  <tab title="project" id="build-config-db-macosx">
    Check out, build and install the project.
    <code-block lang="SHELL" collapsible="false">
<![CDATA[
git clone git@github.com:sptrakesh/config-db.git
cd config-db
cmake -DCMAKE_PREFIX_PATH=/usr/local/boost \
  -DCMAKE_PREFIX_PATH=/usr/local/rocksdb \
  -DCMAKE_PREFIX_PATH=/usr/local/flatbuffers \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local/spt \
  -S . -B build
cmake --build build -j12
sudo cmake --install build
]]>
    </code-block>
  </tab>
</tabs>

## Windows

Install pre-requisites for building and running on **Windows**. The following instructions 
at times reference `arm` or `arm64` architecture. Modify those values as appropriate for 
your hardware. These instructions are based on steps I followed to set up the project on a 
**Windows 11** virtual machine running via **Parallels Desktop** on a **M2** Mac.

### Issues
At present final linking of the service fails with unresolved external symbols. These have 
not manifested themselves on **Mac OS X**. I will update here if I finally manage to 
resolve the linking problems.

* `rocksdb::Cleanable::Cleanable(void)`
* `rocksdb::PinnableSlice::PinnableSlice(class rocksdb::PinnableSlice &&)`
* `rocksdb::Status::ToString(void)const`
* `rocksdb::LRUCacheOptions::MakeSharedCache(void)const`
* `rocksdb::ColumnFamilyOptions::OptimizeForPointLookup(unsigned __int64)`
* `rocksdb::ColumnFamilyOptions::ColumnFamilyOptions(void)`
* `rocksdb::DBOptions::OptimizeForSmallDb(class std::shared_ptr<class rocksdb::Cache> *)`
* `rocksdb::DBOptions::DBOptions(void)`
* `rocksdb::TransactionDB::Open(struct rocksdb::DBOptions const &,struct rocksdb::TransactionDBOptions const &,class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > const &,class std::vector<struct rocksdb::ColumnFamilyDescriptor,class std::allocator<struct rocksdb::ColumnFamilyDescriptor> > const &,class std::vector<class rocksdb::ColumnFamilyHandle *,class std::allocator<class rocksdb::ColumnFamilyHandle *> > *,class rocksdb::TransactionDB **)`
* `rocksdb::kDefaultColumnFamilyName`

### Build
Install dependencies and build the project.

<tabs id="config-db-windows">
  <tab title="Boost" id="build-boost-windows">
    <include from="boost.topic" element-id="boost-windows"/>
  </tab>
  <tab title="fmt" id="build-fmt-windows">
    <include from="build-ranges.topic" element-id="build-fmt-windows"/>
  </tab>
  <tab title="range-v3" id="build-ranges-windows">
    <include from="build-ranges.topic" element-id="build-ranges-windows"/>
  </tab>
  <tab title="flatbuffers" id="build-flatbuffers-windows">
    <code-block lang="PowerShell" collapsible="false">
cd \opt\src
git clone -b v23.5.9 https://github.com/google/flatbuffers.git
cd flatbuffers
cmake -DFLATBUFFERS_BUILD_TESTS=OFF -DFLATBUFFERS_BUILD_CPP17=ON -DFLATBUFFERS_ENABLE_PCH=ON -DCMAKE_PREFIX_PATH=\opt\local -DCMAKE_INSTALL_PREFIX=\opt\local -S . -B build
cmake --build build --target install -j8
cd ..
del /s /q flatbuffers
rmdir /s /q flatbuffers
    </code-block>
  </tab>
  <tab title="vcpkg" id="build-vcpkg-windows">
    <code-block lang="PowerShell" collapsible="false">
cd \opt\src
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat -disableMetrics
.\vcpkg integrate install --vcpkg-root \opt\src\vcpkg
.\vcpkg install openssl:arm64-windows
.\vcpkg install snappy:arm64-windows
.\vcpkg install rocksdb:arm64-windows
.\vcpkg install readline:arm64-windows
    </code-block>
  </tab>
  <tab title="gflags" id="build-gflags-windows">
    Install gflags if compiling RocksDB locally.
    <code-block lang="PowerShell" collapsible="false">
cd \opt\src
git clone https://github.com/gflags/gflags.git
cd gflags
cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DBUILD_TESTING=OFF -DBUILD_gflags_LIBS=ON -DINSTALL_HEADERS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=\opt\local -DCMAKE_INSTALL_PREFIX=\opt\local -S . -B cmake-build
cmake --build cmake-build --target install -j8
cd ..
del /s /q gflags
rmdir /s /q gflags
    </code-block>
  </tab>
  <tab title="RocksDb" id="build-rocksdb-windows">
    <code-block lang="PowerShell" collapsible="false">
cd \opt\src
git clone -b v8.3.2 https://github.com/facebook/rocksdb.git
cd rocksdb
cmake -DWITH_TESTS=OFF -DWITH_ALL_TESTS=OFF -DWITH_BENCHMARK_TOOLS=OFF -DCMAKE_CXX_STANDARD=20 -DCMAKE_BUILD_TYPE=Release -DROCKSDB_BUILD_SHARED=OFF -DCMAKE_PREFIX_PATH=\opt\local -DCMAKE_INSTALL_PREFIX=\opt\local -DCMAKE_TOOLCHAIN_FILE="C:/opt/src/vcpkg/scripts/buildsystems/vcpkg.cmake" -S . -B build -G"Unix Makefiles" -DCMAKE_MAKE_PROGRAM=nmake
set CL=/MP
cmake --build build
cd ..
del /s /q rocksdb
rmdir /s /q rocksdb
    </code-block>
  </tab>
  <tab title="project" id="build-project-windows">
    Launch the Visual Studio Command utility.<br/><strong>Note:</strong> I have not had any success attempting to build <code>nghttp2-asio</code> on Windows ARM64, hence the <code>-DHTTP_SERVER=OFF</code> option is required when invoking cmake.
    <code-block lang="PowerShell" collapsible="false">
cd %\homepath%\source\repos
git clone https://github.com/sptrakesh/config-db.git
cd config-db
cmake -DCMAKE_PREFIX_PATH=\opt\local -DCMAKE_INSTALL_PREFIX=\opt\spt -DCMAKE_TOOLCHAIN_FILE="C:/opt/src/vcpkg/scripts/buildsystems/vcpkg.cmake" -DHTTP_SERVER=Off -S . -B build
cmake --build build --target install -j8
    </code-block>
  </tab>
</tabs>

## API Usage
The [API](https://github.com/sptrakesh/config-db/blob/master/src/api/api.h) can be used 
to communicate with the TCP service. Client code bases can use cmake to use the library.

```cmake
find_package(ConfigDb REQUIRED COMPONENTS api)
if (APPLE)
  include_directories(/usr/local/spt/include)
else()
  include_directories(/opt/spt/include)
endif (APPLE)
target_link_libraries(${Target_Name} PRIVATE configdb::api ...)
```

```shell
# Run cmake
cmake -DCMAKE_PREFIX_PATH="/usr/local/boost;/usr/local/spt" -S . -B build
cmake --build build --parallel
sudo cmake --install build
```