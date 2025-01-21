# Encrypter
Simple applications for encrypting or decrypting values using AES-256-CBC.
Utility applications and a TCP/IP service are available.

## Service
A simple TCP service for providing encrypt/decrypt services to client applications.

<code-block lang="shell" collapsible="true">
<![CDATA[
$ <path to>/encrypter-service --help
usage:
  encrypter-service  options

where options are:
  -?, -h, --help            display usage information
  -c, --console             Log to console (default false)
  -p, --port <2030>         Port on which to listen for tcp traffic (default
                            2030)
  -n, --threads <8>         Number of server threads to spawn (default
                            system)
  -l, --log-level <info>    Log level to use [debug|info|warn|critical]
                            (default info).
  -o, --log-dir <logs/>     Log directory (default logs/)
]]>
</code-block>

## Utilities
Utility applications to use the encryption/decryption utilities are provided.

### CLI
A simple `encrypter` application is provided to encrypt/decrypt text.

```shell
$ <path to>/encrypter -e "some text to encrypt"
R+G83PC2MsqDZnhhul9JlUEUMaEsW/C2WiafZ7a2QVY=
$ <path to>/encrypter -d R+G83PC2MsqDZnhhul9JlUEUMaEsW/C2WiafZ7a2QVY=
some text to encrypt
```

### Shell
A simple `encryptsh` application to perform multiple encrypt/decrypt operations in
a single session.

<code-block lang="shell" collapsible="true">
<![CDATA[
$ <path to>/encryptsh
Enter commands followed by <ENTER>
Enter help for help about commands
Enter exit or quit to exit shell
encrypter> help
Available commands
  enc <value> - Encrypt the value.  Eg. [enc some words with . and "quotes"]
  dec <encrypted value> - Decrypt the value.  Eg. [dec haxRkxWhpzHeWnnlynSEqq9Lvvs4pJ3BkfAWU0VXwbc=]
encrypter> enc some text to encrypt with "quotes" and (parantheses)
R+G83PC2MsqDZnhhul9JlUFOSydGVDoE0snZn3c57148Y6LBZddRkdKOQwoggCVDxkZkdNxr0AAesgdshJidlQ==
encrypter> dec R+G83PC2MsqDZnhhul9JlUFOSydGVDoE0snZn3c57148Y6LBZddRkdKOQwoggCVDxkZkdNxr0AAesgdshJidlQ==
some text to encrypt with "quotes" and (parantheses)
encrypter> exit
Bye
]]>
</code-block>

## Build
Standard `cmake` build system.  Main dependency is on [boost](https://boost.org/).

<code-block lang="shell" collapsible="true">
git clone https://github.com/sptrakesh/encrypter.git
cd encrypter
cmake -DCMAKE_PREFIX_PATH=/usr/local/boost \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local/spt \
  -DOPENSSL_ROOT_DIR=/opt/homebrew/opt/openssl \
  -S . -B build
cmake --build build -j12
sudo cmake --install build
</code-block>

### API Usage
The [API](https://github.com/sptrakesh/encrypter/blob/master/src/api/api.hpp) can be used to communicate with the encrypter TCP service.  Client code bases
can use cmake to use the library.

<code-block lang="shell" collapsible="true">
# In your CMakeLists.txt file
find_package(Encrypter REQUIRED COMPONENTS api)
target_link_libraries(${Target_Name} PRIVATE encrypter::api ...)

# Run cmake
cmake -DCMAKE_PREFIX_PATH=/usr/local/boost -DCMAKE_PREFIX_PATH=/usr/local/spt -S . -B build
cmake --build build -j12
</code-block>

<code-block lang="c++" collapsible="true">
<![CDATA[
#include <encrypter/api/api.hpp>

int main()
{
  const auto text = "Test string to be encrypted and decrypted"sv;
  auto enc = api::encrypt( text );
  const auto dec = api::decrypt( enc );
}
]]>
</code-block>

## Docker
Docker image with the service and utility is [available](https://hub.docker.com/repository/docker/sptrakesh/encrypter).
Use the image to either encrypt/decrypt once, or run as a service that interested
applications can use to encrypt/decrypt as required.

### Running CLI
The image can be used to encrypt or decrypt via single runs of the container
as follows:

```shell
$ docker run -it --rm --log-driver=none encrypter -e 'test words'
u2QlLe4TyhY9wUM7BmCQaw==
$ docker run -it --rm --log-driver=none encrypter -d 'u2QlLe4TyhY9wUM7BmCQaw=='
test words
```

### Running Service
To run the container with the TCP/IP service specify the `RUN_SERVER` environment
variable.

```shell
docker run -d --rm -p 2030:2030 -e "RUN_SERVER=true" -e "LOG_LEVEL=debug" --name encrypter encrypter
```
