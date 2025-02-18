# nghttp2-asio

Fork of the [nghttp2-asio](https://github.com/nghttp2/nghttp2-asio) project.  The project is
no longer being maintained (see list of pull requests that have been pending since 2022).  I
have merged all the outstanding PR's as well as made some changes (breaking) to the interfaces
to suit my purposes (use boost 1.87 for instance).

In particular, the pool of `boost::asio::io_context` instances have been replaced with the
standard model of using a single `io_context` instance that is *run* on the desired number of
threads.  To ensure thread-safety, a *strand* per connection approach is adopted.

## Build
Instructions for building the library on popular platforms.

### Mac OS X
Install dependencies to build the project.

#### Boost {collapsible="true" id="nghttp2::macosx::boost"}
<include from="boost.topic" element-id="boost-macosx"/>

#### libnghttp2 {collapsible="false" id="nghttp2::macosx::lib"}
```shell
brew install -y libnghttp2
```

### Build project {collapsible="true" id="nghttp2::macosx::project"}
Check out sources from the fork and build.  If you wish to run the tests or the examples
turn the appropriate options to `ON` from `OFF`.

```Shell
cd /tmp
git clone --branch dev https://github.com/sptrakesh/nghttp2-asio.git
cd nghttp2-asio
cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/usr/local/boost \
  -DCMAKE_INSTALL_PREFIX=/usr/local/nghttp2 \
  -DBUILD_EXAMPLE=OFF -DBUILD_TESTING=OFF \
  -S . -B build
cmake --build build -j12
sudo cmake --install build

cd ..
rm -rf nghttp2-asio
```

### Linux
Install dependencies to build the project.

#### Boost {collapsible="true" id="nghttp2::linux::boost"}
* Download and extract Boost 1.86 (or above) to a temporary location (eg. \opt\src).
* Launch the Visual Studio Command utility and cd to the temporary location.

```shell
cd /tmp
curl -OL https://archives.boost.io/release/1.87.0/source/boost_1_87_0.tar.bz2
tar xfj boost_1_87_0.tar.gz
cd boost_1_87_0
./bootstrap.bat
./b2 -j8 install threading=multi address-model=64 architecture=arm asynch-exceptions=on --prefix=/opt/local --without-python --without-mpi

cd ..
rm -rf boost_1_87_0*
```

#### nghttp2 {collapsible="true" id="nghttp2::linux::lib"}
Check out the latest sources for `nghttp2` and install.

```shell
cd /tmp
NGHTTP_VERSION='1.64.0'
curl -O -L https://github.com/nghttp2/nghttp2/releases/download/v${NGHTTP_VERSION}/nghttp2-${NGHTTP_VERSION}.tar.xz
tar xf nghttp2-${NGHTTP_VERSION}.tar.xz
cd nghttp2-${NGHTTP_VERSION}
./configure --prefix=/opt/local --enable-lib-only --enable-shared=no
make -j8
sudo make install
cd ..
rm -rf nghttp2-${NGHTTP_VERSION}*
```

### Build project {collapsible="true" id="nghttp2::linulinux::project"}
Check out sources from the fork and build.  If you wish to run the tests or the examples
turn the appropriate options to `ON` from `OFF`.  The default values for the options
are `OFF`, so you can omit both options as well.

```Shell
cd /tmp
git clone --branch dev https://github.com/sptrakesh/nghttp2-asio.git
cd nghttp2-asio
cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/opt/local \
  -DCMAKE_INSTALL_PREFIX=/opt/local \
  -DBUILD_EXAMPLE=OFF -DBUILD_TESTING=OFF \
  -S . -B build
cmake --build build -j12
sudo cmake --install build

cd ..
rm -rf nghttp2-asio
```

### Windows
Install dependencies to build the project. The following instructions at times reference `arm` or 
`arm64` architecture. Modify those values as appropriate for your hardware. These instructions
are based on steps I followed to set up the project on a *Windows 11* virtual machine running
via Parallels Desktop on a M2 Mac.

> Project does not (yet) build on Windows. I have seen some posts claiming it does, but I have
> not succeeded in getting it to build.

#### Boost {collapsible="true" id="nghttp2::windows::boost"}
* Download and extract Boost 1.86 (or above) to a temporary location (eg. \opt\src).
* Launch the Visual Studio Command utility and cd to the temporary location.

```powershell
cd \opt\src
curl -OL https://archives.boost.io/release/1.87.0/source/boost_1_87_0.tar.bz2
tar -xfz boost_1_87_0.tar.gz
cd boost_1_87_0
.\bootstrap.bat
.\b2 -j8 install threading=multi address-model=64 architecture=arm asynch-exceptions=on --prefix=\opt\local --without-python --without-mpi

cd ..
del /s /q boost_1_87_0
rmdir /s /q boost_1_87_0
```

#### vckg {collapsible="true"}
Launch the Visual Studio Command utility.

```powershell
cd \opt\src
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat -disableMetrics
.\vcpkg integrate install --vcpkg-root \opt\src\vcpkg
.\vcpkg install openssl:arm64-windows
.\vcpkg install nghttp2:arm64-windows
```

#### Sources {collapsible="true"}
Launch the Visual Studio Command utility.

```powershell
cd \%homepath\%\source\repos
git clone https://github.com/sptrakesh/nghttp2-asio.git
cd nghttp2-asio
git checkout dev
cmake -DCMAKE_PREFIX_PATH=\opt\local;\opt\src\vcpkg\packages\nghttp2_arm64-windows;\opt\src\vcpkg\packages\openssl_arm64-windows -DBUILD_EXAMPLE=ON -DBUILD_TESTING=ON -DBOOST_STATIC_LIBS=OFF -S . -B build
cmake --build build -j8
```
