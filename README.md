# Barebones webserver

## Summary

This is like a proof of concept of a basic webserver, but with a more unconventional style of request handling. The following steps summarize the basic behavior:

1. Mainthread runs a select()-based eventloop
2. When a client connections, the mainthread parses the input and submits a pointer/ref of the http request to a threadsafe queue
3. A threadpool consumes this queue and processes the requests
4. After processing, a thread pipes a signal to the mainthread when it is done processing
5. Eventually, the eventloop closes the client connection and does the necessary cleanup

Some code parts are inspired by beejs guide for network programming (https://beej.us/guide/bgnet/)

## How to build

### Depedencies

Fetch the dependency into the directory of this README file and unzip:

wget https://github.com/h2o/picohttpparser/archive/refs/tags/v1.2.zip && unzip v1.2.zip

### Build and launch the servr at port 3000

make && ./dualserver

## Benchmarking

I did some benchmarking with apache benchmark (ab), but just basic functionality testing.
I'd be interested in how the server performs under high concurrency. Parameters in the code are not optimized at all though.

b -n 1000 -c 100 127.0.1:3000/

## Ideas

* pre memory allocation for http-requests, instead of calling malloc() repeatedly

* replace select() with epoll()/kqueue() or io_uring

* lockfree queue for http-requests

* config file

* store handler mappings in a tree structure for faster search

* ...
