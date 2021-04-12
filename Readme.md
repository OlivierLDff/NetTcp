# NetTcp

NetTcp provide a Tcp `Server` that create Tcp `Socket` for each incoming connections for server side.

A `Socket` can also be used on client side to connect to a remote server.

## 🚀 Overview

<p align="center">
  <img src="./docs/ClassDiagram.svg"/>
</p>

Checkout the documentation to learn how to use **NetTcp**:

* Learn how to create a [Client](https://olivierldff.github.io/NetTcp/GettingStart.html#create-a-client) and a [Server](https://olivierldff.github.io/NetTcp/GettingStart.html#create-a-server).
* Checkout [Examples](https://olivierldff.github.io/NetTcp/GettingStart.html#examples).
* [Use the library with QML](https://olivierldff.github.io/NetTcp/QmlUsage.html).
* [Build the library](https://olivierldff.github.io/NetTcp/BuildAndRun.html).

## 📌 Dependencies

**NetTcp** relies on:

* Multiple libraries from the Qt framework.
  * Qt::Core
  * Qt::Network
  * Qt::Qml
* **QOlm** to present the **Server** as a list of **Socket**.
* **spdlog** to provide customizable log sink.

![dependencies](./docs/dependencies.svg)

## Authors

* [Olivier Le Doeuff](https://github.com/OlivierLDff)