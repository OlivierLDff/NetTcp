---
layout: default
title: 🔨 Build & Tests
nav_order: 3
---

# Build and Run

## Dependencies

* The library depends on C++ 14 STL.
* [QOlm](https://github.com/OlivierLDff/QOlm.git) that make look `Server` like a list of `Socket`.
* [spdlog](https://github.com/gabime/spdlog) for logging.
* Qt Core and Network for the backend.
* Qml dependencies:
  * Qt Qml Quick Control2
  * [Stringify](https://github.com/OlivierLDff/Stringify)
  * [Qaterial](https://github.com/OlivierLDff/Qaterial)

## Tools

* [CMake](https://cmake.org/) v3.14 or greater.
* C++14 compliant compiler or greater.
* Internet connection to download dependencies during configuration.

This library use CMake for configuration.

```bash
git clone https://github.com/Naostage/NetTcp
cd NetTcp
mkdir build && cd build
cmake ..
```

The `CMakeLists.txt` will download every dependencies for you.

## Building

Simply use integrated cmake command:

```bash
cmake --build . --config "Release"
```

## Execute Examples

```bash
cmake -DNETTCP_ENABLE_EXAMPLES=ON ..
cmake --build . --target NetTcp_EchoClientServer
./NetTcp_EchoClientServer
```

Available examples are :
* `NetTcp_EchoClientServer` : Demonstrate a client that a string to a server. Server answer with the same string.
* `NetTcp_EchoClient`: Only the client part of `NetTcp_EchoClientServer`.
* `NetTcp_EchoServer`: Only the server part of `NetTcp_EchoClientServer`.`NetTcp_FuzzDisconnectionClientServer`: Send error string from client to server, and test that server handle ok the disconnection. This help to profile memory leaks and thread issues. (run with `-t`).
* `NetTcp_FuzzDisconnectionServerClient`: Reply error string from server to client.

## Additional CMake flags

Since CMake is using `FetchContent` functionality, you can add flags to understand what is going on. The library also require Qt, so you need to indicate where Qt SDK is installed. Provide the path with `CMAKE_PREFIX_PATH`.

```bash
cmake
# Log output during download of dependencies
-DFETCHCONTENT_QUIET=OFF
# Avoid that dependencies source gets pulled at each cmake command
# Very useful when developping on dependencies too.
-DFETCHCONTENT_UPDATES_DISCONNECTED=ON
# Add path to qt sdk(required if you system qt is lower than Qt 5.12)
-DCMAKE_PREFIX_PATH=/Path/To/Qt
..
```

## Integrating

Adding NetTcp library in your library is really simple if you use CMake 3.14.

In your `CMakeLists.txt`:

```cmake
# ...
include(FetchContent)
FetchContent_Declare(
    NetTcp
    GIT_REPOSITORY "https://github.com/OlivierLDff/NetTcp"
    GIT_TAG        "master"
)
# ...
FetchContent_MakeAvailable(NetTcp)
# ...

target_link_libraries(MyTarget PUBLIC NetTcp)
```

Then you just need to `#include <Net/Tcp/NetTcp.hpp>`.