# CppServer

[![Linux build status](https://img.shields.io/travis/chronoxor/CppServer/master.svg?label=Linux)](https://travis-ci.org/chronoxor/CppServer)
[![OSX build status](https://img.shields.io/travis/chronoxor/CppServer/master.svg?label=OSX)](https://travis-ci.org/chronoxor/CppServer)
[![MinGW build status](https://img.shields.io/appveyor/ci/chronoxor/CppServer/master.svg?label=MinGW)](https://ci.appveyor.com/project/chronoxor/CppServer)
[![Windows build status](https://img.shields.io/appveyor/ci/chronoxor/CppServer/master.svg?label=Windows)](https://ci.appveyor.com/project/chronoxor/CppServer)

C++ Server Library provides functionality to create different kind of
client/server solutions.

[CppServer API reference](http://chronoxor.github.io/CppServer/index.html)

# Contents
  * [Features](#features)
  * [Requirements](#requirements)
  * [How to build?](#how-to-build)
    * [Clone repository with submodules](#clone-repository-with-submodules)
    * [Linux](#linux)
    * [OSX](#osx)
    * [Windows (MinGW)](#windows-mingw)
    * [Windows (MinGW with MSYS)](#windows-mingw-with-msys)
    * [Windows (Visaul Studio 2015)](#windows-visaul-studio-2015)
  * [OpenSSL certificates](#openssl-certificates)

# Features
* Cross platform
* Benchmarks
* Examples
* Tests
* [Doxygen](http://www.doxygen.org) API documentation
* Continuous integration ([Travis CI](https://travis-ci.com), [AppVeyor](https://www.appveyor.com))

# Requirements
* Linux
* OSX
* Windows 7 / Windows 10
* [CMake](http://www.cmake.org)
* [GIT](https://git-scm.com)
* [GCC](https://gcc.gnu.org)

Optional:
* [Clang](http://clang.llvm.org)
* [Clion](https://www.jetbrains.com/clion)
* [MinGW](http://mingw-w64.org/doku.php)
* [Visual Studio 2015](https://www.visualstudio.com)

#How to build?

## Clone repository with submodules
```
git clone https://github.com/chronoxor/CppServer.git CppServer
cd CppServer
git submodule update --init --recursive --remote
```

## Linux
```
cd build
./unix.sh
```

## OSX
```
cd build
./unix.sh
```

## Windows (MinGW)
```
cd build
mingw.bat
```

## Windows (Visaul Studio 2015)
```
cd build
vs.bat
```

# OpenSSL certificates
In order to create OpenSSL based server and client you should prepare a set of
SSL certificates. Here comes several steps to get a self-signed set of SSL
certificates for testing purposes:

1) Generate a 4096-bit long RSA key for root CA and store it in a file:

```
openssl genrsa -out ca-key.pem 4096
```

If you want to password-protect this key, add option -des3

2) Create self-signed root CA certificate. You’ll need to provide an identity
for your root CA:

```
openssl req -new -x509 -nodes -days 3652 -key ca-key.pem -out ca-cert.pem
```

The -x509 option is used for a self-signed certificate. 3652 days gives us a
cert valid for 5 years.

3) Generate a 4096-bit long RSA key for a new server CA and request a server
certificate signed by the root CA:

```
openssl req -newkey rsa:4096 -days 3652 -nodes -keyout server-key.pem -out server-req.pem
openssl x509 -req -in server-req.pem -days 3652 -CA ca-cert.pem -CAkey ca-key.pem -set_serial 01 -out server-cert.pem
```

Make sure that the Common Name you enter here is different from the
Common Name you entered previously for the root CA. If they are the same,
you will get an error later on when creating the pkcs12 file.

4) Generate a 4096-bit long RSA key for a new client CA and request a client
certificate signed by the root CA:

```
openssl req -newkey rsa:4096 -days 3652 -nodes -keyout client-key.pem -out client-req.pem
openssl x509 -req -in client-req.pem -days 3652 -CA ca-cert.pem -CAkey ca-key.pem -set_serial 01 -out client-cert.pem;
```
