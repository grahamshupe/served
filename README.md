# Served

Served is a lightweight, performant HTTP/1.1 web server with optional library functionality.

## Getting Started

### Building

Served currently only supports Linux systems. To build it, simply clone this repo and run the Makefile, assuming you have GCC and Make installed:

```
make
```

### Usage

While Served can be used as a library, it currently lacks full support and documentation, so it is recommended to use it as a standalone server.

Run the Served file to start the standalone server:

```
./served [arguments]
```

Served supports several arguments at startup, which can help fine tune the server:

* -r [path] specifies the root directory which Served will serve files from. If not specified, this defaults to test/root/

## Goals

Served is not and will never be a replacement for other, more robust web server software such as [nginx](https://nginx.org/) or HTTP libraries like [LibCurl](
https://curl.se/libcurl/). Instead, this is a passion project created primarily for my own exploration and learning. 

That said, the overarching goals of this project are as follows:

1. Create a functional, modular program capable of sending and recieving GET requests.
2. Achieve performance and stess tolerance comparable to industry-leading server software such as nginx.
3. Handle sockets at a low level, using system calls such as epoll to increase performance.
4. Strictly adhere to internet standards as defined in the IETF's [RFC 9112](https://datatracker.ietf.org/doc/html/rfc9112) and [RFC 9110](https://datatracker.ietf.org/doc/html/rfc9110)

Considering the scope of this project, I do not include safety or security as a goal. This is not to say that security was not considered while I made Served. In fact, many parts of the code are constructed to meticulously avoid buffer overflows and other security flaws. Rather, I simply cannot guarantee that Served will provide even a reason amount of security against attacks, and I do not expect it to.


