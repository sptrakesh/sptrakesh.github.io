# Build
Check out the sources and use `cmake` to build and install the project locally.

## UNIX
<tabs id="mongo-service-unix">
  <tab title="Boost" id="build-boost-unix">
    <include from="boost.topic" element-id="boost-unix"/>
  </tab>
  <tab title="mongocxx" id="build-mongocxx-unix">
    Install <a href="https://www.mongodb.com/docs/languages/cpp/">mongocxx</a> driver.
    <code-block lang="SHELL" collapsible="false">
export PREFIX=/opt/local
    </code-block>
    <include from="build-mongocxx.topic" element-id="build-mongocxx-unix"/>
  </tab>
  <tab title="project" id="build-mongo-service-unix">
    Check out, build and install the project.
    <code-block lang="SHELL" collapsible="true">
<![CDATA[
cd /tmp
git clone https://github.com/sptrakesh/mongo-service.git
cd mongo-service
cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/opt/local \
  -DCMAKE_INSTALL_PREFIX=/opt/spt \
  -DBUILD_TESTING=OFF -S . -B build
cmake --build build -j12
sudo cmake --install build
]]>
    </code-block>
  </tab>
</tabs>

## Mac OS X
<tabs id="mongo-service-macosx">
  <tab title="Boost" id="build-boost-macosx">
    <include from="boost.topic" element-id="boost-macosx"/>
  </tab>
  <tab title="mongocxx" id="build-mongocxx-macosx">
    Install <a href="https://www.mongodb.com/docs/languages/cpp/">mongocxx</a> driver.
    <code-block lang="SHELL" collapsible="false">
export PREFIX=/usr/local/mongo
    </code-block>
    <include from="build-mongocxx.topic" element-id="build-mongocxx-unix"/>
  </tab>
  <tab title="project" id="build-mongo-service-macosx">
    Check out, build and install the project.
    <code-block lang="SHELL" collapsible="true">
<![CDATA[
cd /tmp
git clone https://github.com/sptrakesh/mongo-service.git
cd mongo-service
cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/usr/local/boost \
  -DCMAKE_PREFIX_PATH=/usr/local/mongo \
  -DCMAKE_INSTALL_PREFIX=/opt/spt \
  -DBUILD_TESTING=OFF -S . -B build
cmake --build build -j12
sudo cmake --install build
]]>
    </code-block>
  </tab>
</tabs>

## Windows

Install dependencies to build the project.  The following instructions at times reference `arm` or `arm64` architecture.  Modify
those values as appropriate for your hardware.  These instructions are based on steps I followed to set up the project on a
Windows 11 virtual machine running via Parallels Desktop on a M2 Mac.

<tabs id="mongo-service-windows">
  <tab title="Boost" id="build-boost-windows">
    <include from="boost.topic" element-id="boost-windows"/>
  </tab>
  <tab title="mongocxx" id="build-mongocxx-windows">
    Install <a href="https://www.mongodb.com/docs/languages/cpp/">mongocxx</a> driver.
    <include from="build-mongocxx.topic" element-id="build-mongocxx-windows"/>
  </tab>
  <tab title="fmt" id="build-fmt-windows">
    <include from="build-ranges.topic" element-id="build-fmt-windows"/>
  </tab>
  <tab title="ranges" id="build-ranges-windows">
    <include from="build-ranges.topic" element-id="build-ranges-windows"/>
  </tab>
  <tab title="vcpkg" id="install-vcpkg-windows">
    Install <a href="https://github.com/Microsoft/vcpkg">vcpkg</a> manager.  Launch the Visual Studio Command utility.
    <code-block lang="PowerShell" collapsible="true">
cd \opt\src
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat -disableMetrics
.\vcpkg integrate install --vcpkg-root \opt\src\vcpkg
.\vcpkg install curl:arm64-windows
.\vcpkg install cpr:arm64-windows
    </code-block>
  </tab>
  <tab title="project" id="build-mongo-service-windows">
    Check out, build and install the project.
    <code-block lang="PowerShell" collapsible="true">
<![CDATA[
cd %\homepath%\source\repos
git clone https://github.com/sptrakesh/mongo-service.git
cd mongo-service
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=\opt\local -DCMAKE_INSTALL_PREFIX=\opt\spt -DBUILD_TESTING=ON -DCMAKE_TOOLCHAIN_FILE="C:/opt/src/vcpkg/scripts/buildsystems/vcpkg.cmake" -S . -B build
cmake --build build -j8
cmake --build build --target install
]]>
    </code-block>
  </tab>
</tabs>