**NOTE** Currently this repo contains *RESTinio* version that is under development.

[TOC]

# What Is It?
*RESTinio* is a header-only library for creating REST applications in c++.
It helps to create http server that can handle requests asynchronously.
Currently it is in beta state and represents our solution for the problem of
being able to handle request asynchronously with additional features.

*RESTinio* is a free software and is distributed under GNU Affero GPL v.3 license.

## Why creating yet another library of that kind?

Well, there are lots of libraries and frameworks
of all sorts of complexity and maturity for
building REST service in C++.
Isn't it really a [NIH syndrom](https://en.wikipedia.org/wiki/Not_invented_here)?

We've used some of already available libraries and have tried lots of them.
And we have found that pretty much of them lack an ability
to handle requests asynchronously.
Usually library design forces user to set the response
inside a handler call. So when handling needs some interactions
with async API such design results in blocking of a caller thread.
And that hurts when the rest of the application is built on async bases.

In addition to async handling feature we though it would be nice
for such library to keep track of what is going on with connections and
control timeouts on operations of reading request from socket,
handling request and writing response to socket.
And it would also be nice to have request handler router
(like in [express](https://expressjs.com/)).
And a header-only design is a plus.

And it happens that under such conditions you don't have a lot of options.
So we have come up with *RESTinio*...

# Obtain And Build

## Prerequisites

To use *RESTinio* it is necessary to have:

* Reasonably modern C++14 compiler (VC++14.0, GCC 5.2 or above, clang 3.6 or above);
* [asio](http://think-async.com/Asio) 1.11.0;
* [nodejs/http-parser](https://github.com/nodejs/http-parser) 2.7.1 or above;
* [fmtlib](http://fmtlib.net/latest/index.html) 3.0.1 or above.
* Optional: [SObjectizer](https://sourceforge.net/projects/sobjectizer/) 5.5.18 and above;

For building samples, benchmarks and tests:

* [Mxx_ru](https://sourceforge.net/projects/mxxru/) 1.6.11 or above;
* [rapidjson](https://github.com/miloyip/rapidjson) 1.1.0;
* [json_dto](https://bitbucket.org/sobjectizerteam/json_dto-0.1) 0.1.2.1 or above;
* [args](https://github.com/Taywee/args) 6.0.4;
* [CATCH](https://github.com/philsquared/Catch) 1.8.2 or above.

## Obtaining

There are two ways of obtaining *RESTinio*.

* Getting from
[repository](https://bitbucket.org/sobjectizerteam/restinio-0.2).
In this case external dependencies must be obtained with Mxx_ru externals tool.
* Getting
[archive](https://bitbucket.org/sobjectizerteam/restinio-0.2/downloads/restinio-0.2.0-full.tar.bz2).
Archive includes source code for all external dependencies.

### Cloning of hg repository

```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.2
```

And then:
```
cd restinio-0.2
mxxruexternals
```
to download and extract *RESTinio*'s dependencies.

### MxxRu::externals recipe

See MxxRu::externals recipes for *RESTinio*
[here](./doc/MxxRu_externals_recipe.md).

### Getting archive

```
wget https://bitbucket.org/sobjectizerteam/restinio-0.2/downloads/restinio-0.2.0-full.tar.bz2
tar xjvf restinio-0.2.0-full.tar.bz2
cd restinio-0.2.0-full
```

## Build

### CMake

Building with CMake currently is provided for samples, tests and benches
not depending on SObjectizer.
To build them run the following commands:
```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.2
cd restinio-0.2
mxxruexternals
cd dev
mkdir cmake_build
cd cmake_build
cmake -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

Or, if getting sources from archive:
```
wget https://bitbucket.org/sobjectizerteam/restinio-0.2/downloads/restinio-0.2.0-full.tar.bz2
tar xjvf restinio-0.2.0-full.tar.bz2
cd restinio-0.2.0-full/dev
mkdir cmake_build
cd cmake_build
cmake -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

### Mxx_ru
While *RESTinio* is header-only library, samples, tests and benches require a build.

Compiling with Mxx_ru:
```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.2
cd restinio-0.2
mxxruexternals
cd dev
ruby build.rb
```

For release or debug builds use the following commands:
```
ruby build.rb --mxx-cpp-release
ruby build.rb --mxx-cpp-debug
```

*NOTE.* It might be necessary to set up `MXX_RU_CPP_TOOLSET` environment variable,
see Mxx_ru documentation for further details.

### Dependencies default settings

External libraries used by *RESTinio* have the following default settings:

* A standalone version of *asio* is used and a chrono library is used,
so `ASIO_STANDALONE` and `ASIO_HAS_STD_CHRONO` defines are necessary;
* a less strict version of *nodejs/http-parser* is used, so the following
definition `HTTP_PARSER_STRICT=0` is applied;
* *fmtlib* is used as a header-only library, hence a `FMT_HEADER_ONLY`
define is necessary;

# How To Use It?

## Getting started

To start using *RESTinio* make sure that all dependencies are available.
The tricky one is [nodejs/http-parser](https://github.com/nodejs/http-parser),
because it is a to be compiled unit, which can be built as a static library and
linked to your target or can be a part of your target.

To start using *RESTinio* simply include `<restinio/all.hpp>` header.
It includes all necessary headers of the library.

It easy to learn how to use *RESTinio* by example.
Here is a hello world application
([see full example](./dev/sample/hello_world_basic/main.cpp)):
~~~~~
::c++
using http_server_t = restinio::http_server_t<>;

http_server_t http_server{
  restinio::create_child_io_service( 1 ),
  []( auto & settings ){
    settings.port( 8080 ).request_handler( request_handler() );
  }
};

// Start server.
http_server.open();

// ...

// Stop server.
http_server.close();
~~~~~

Template class `restinio::http_server_t<TRAITS>` encapsulates server
logic. It has two parameters: the first one is a wrapper for
`asio::ioservice` instance passed as
`io_service_wrapper_unique_ptr_t`, and the second one is a
`server_settings_t<TRAITS>` object that defines
server port, protocol (ipv4/ipv6), timeouts etc.

*RESTinio* does its networking stuff with
[asio](http://think-async.com/Asio) library, so to run server
it must have an `asio::io_service` instance to run on.
Internal logic of *RESTinio* is separated from
maintaining `asio::ioservice` directly by a wrapper class.
In most cases it would be enough to use one of standard
wrappers. The first one is provided by
`use_existing_io_service( asio::io_service & )`
and is a proxy for user managed `asio::io_service` instance.
And the second is the one provided by
`create_child_io_service( unsigned int thread_pool_size )`
that creates an object with `asio::io_service` inside
that runs on a thread pool and is managed by server object.
Child io_service running on a single thread is used in example.

Server settings are set with lambda.
It is more convenient to use generic lambda to omit
naming the concrete type of `server_settings_t<TRAITS>`.
View `server_settings_t` class to see all available params
([settings.hpp](./dev/restinio/settings.hpp)).

To run the server `open()` and `close()` methods are used.
When open method is called server posts a callback on
`asio::ioservice` to bind on a server port and listen for new connections.
If callback executes without errors, open returns, in case of error
it throws. Note that in general open and close operations are executed
asynchronously so to make them sync a future-promise is used.

To make server complete we must set request handler.
Request handler as defined by default `TRATS` is a
`default_request_handler_t`:
~~~~~
::c++
using default_request_handler_t =
  std::function< request_handling_status_t ( request_handle_t ) >
~~~~~

To create handler a `request_handler()` function is used:
~~~~~
::c++
auto request_handler()
{
  return []( auto req ) {
      if( restinio::http_method_get() == req->header().method() &&
        req->header().request_target() == "/" )
      {
        req->create_response()
          .append_header( "Server", "RESTinio hello world server" )
          .append_header_date_field()
          .append_header( "Content-Type", "text/plain; charset=utf-8" )
          .set_body( "Hello world!")
          .done();

        return restinio::request_accepted();
      }

      return restinio::request_rejected();
    };
}
~~~~~

Request handler here is a lambda-function, it checks if request method is `GET`
and target is `/` and if so makes a response and returns
`restinio::request_handling_status_t::accepted`
meaning that request had been taken for processing.
Otherwise handler returns `restinio::request_handling_status_t::rejected` value
meaning that request was not accepted for handling and *RESTinio* must take care of it.

## Basic idea

In general *RESTinio* runs its logic on `asio::io_service`.
There are two major object types running:

* acceptor -- receives new connections and creates connection objects that
performs session logic;
* connection -- does tcp io-operations, http-parsing and calls handler.

There is a single instance of acceptor and as much connections as needed.

Acceptors life cycle is trivial and is the following:

1. Start listening for new connection.
2. Receive new tcp-connection.
3. Create connection handler object and start running it.
4. Back to step 1'.

When the server is closed cycle breaks up.

Connections life cycle is more complicated and cannot be expressed lineary.
Simultaneously connection runs two logical objectives. The first one is
responsible for receiving requests and passing them to handler (read part) and
the second objective is streaming resulting responses back to client (write part).
Such logical separation comes from HTTP pipelining support and
various types of response building strategies.

Without error handling and timeouts control Read part looks like this:

1. Start reading from socket.
2. Receive a portion of data from socket and parse HTTP request out of it.
3. If HTTP message parsing is incomplete then go back to step 1.
4. If HTTP message parsing is complete pass request and connection to request handler.
5. If request handler rejects request, then push not-implemented response (status 501)
to outgoing queue and stop reading from socket.
5. If request was accepted and the number of requests in process is less than
`max_pipelined_requests` then go back to step 1.
6. Stop reading socket until awaken by the write part.

And the Write part looks like this:

1. Wait for response pieces initiated from user domain
either directly inside of handler call or from other context where
response actually is being built.
2. Push response data to outgoing queue with consideration of associated response position
(multiple request can be in process, and response for a given request
cannot be written to socket before writing all previous responses to it).
3. Check if there is outgoing data ready to send.
4. If there is no ready data available then go back to step 1.
5. Send ready data.
6. Wait for write operation to complete. If more response pieces comes while
write operation runs it is simply received (steps 1-2 without any further go).
7. After write operation completes:
if last committed response was marked to close connection
then connection is closed and destroyed.
8. If it appears that the room for more pipeline requests became available again
then awake the read part.
9. Go back to step 3.

Of course implementation has error checks. Also implementation controls timeouts of
operations that are spread in time:

* reading the request: from starting reading bytes from socket up to
parsing a complete http-message;
* handling the request: from passing request data and connection handle
to request handler up to getting response to be written to socket;
* writing response to socket.

When handling a request there are two possible cases:

* response is created inside the request handlers call;
* request handler delegates handling job to other context via
some kind of async API.

The first case is trivial and response is simply begins to be written.

The second case and its possibility is a key point of *RESTinio* being created for.
As request data and connection handle are wrapped in shared pointers
so they can be moved to other context.
So it is possible to create handlers that can interact with async API.
When response data is ready response can be built and sent using request handle.
After response building is complete connection handle
will post the necessary job to run on host `asio::io_service`.
So one can perform asynchronous request handling and
not to block worker threads.

## Class *http_server_t*

`http_server_t` is a template class parameterized with a single
template parameter: `TRAITS`.
Traits class must specify a set types used inside *RESTinio*,
they are:
~~~~~
::c++
timer_factory_t;
logger_t;
request_handler_t;
strand_t;
~~~~~

It is easier to use `restinio::traits_t<>` helper type:
~~~~~
::c++
template <
    typename TIMER_FACTORY,
    typename LOGGER,
    typename REQUEST_HANDLER = default_request_handler_t,
    typename STRAND = asio::strand< asio::executor > >
struct traits_t;
~~~~~

Here is what `TRAITS` member type stand for:

* `timer_factory_t` defines the logic of how timeouts are managed;
* `logger_t` defines logger that is used by *RESTinio* to track
its inner logic;
* `request_handler_t` defines a function-like type to be used as
request handler;
* `strand_t` - defines a class that is used by connection as a wrapper
for its callback-handlers running on `asio::io_service` thread(s)
in order to guarantee serialized callbacks invocation
(see [asio doc](http://think-async.com/Asio/asio-1.11.0/doc/asio/reference/strand.html)).
Actually there are two options for the strand type:
`asio::strand< asio::executor >` and `asio::executor`.

It is handy to consider `http_server_t<TRAITS>` class as a root class
for the rest of *RESTinio* ecosystem, because pretty much all of them are
also template types parameterized with the same `TRAITS` parameter.

Class `http_server_t<TRAITS>` has two constructors:
~~~~~
::c++
http_server_t(
  io_service_wrapper_unique_ptr_t io_service_wrapper,
  server_settings_t<TRAITS> settings );

template < typename CONFIGURATOR >
http_server_t(
  io_service_wrapper_unique_ptr_t io_service_wrapper,
  CONFIGURATOR && configurator )
~~~~~

The first is the main one. It obtains `io_service_wrapper` as an
`asio::io_service` back-end and server settings with the bunch of params.

The second constructor simplifies setting of parameters via generic lambda:
~~~~~
::c++
http_server_t http_server{
  restinio::create_child_io_service( 1 ),
  []( auto & settings ){ // Omit concrete name of settings type.
    settings
      .port( 8080 )
      .read_next_http_message_timelimit( std::chrono::seconds( 1 ) )
      .handle_request_timeout( std::chrono::milliseconds( 3900 ) )
      .write_http_response_timelimit( std::chrono::milliseconds( 100 ) )
      .logger( /* logger params */ )
      .request_handler( /* request handler params */ );
  } };
~~~~~

### Running server

To run server there are open()/close() methods:
~~~~~
::c++
template <
    typename SRV_OPEN_OK_CALLBACK,
    typename SRV_OPEN_ERR_CALLBACK >
void
open_async(
  SRV_OPEN_OK_CALLBACK && open_ok_cb,
  SRV_OPEN_ERR_CALLBACK && open_err_cb );

void
open_sync();

// Shortcut for open_sync().
void
open();

template <
    typename SRV_CLOSE_OK_CALLBACK,
    typename SRV_CLOSE_ERR_CALLBACK >
void
close_async(
  SRV_CLOSE_OK_CALLBACK && close_ok_cb,
  SRV_CLOSE_ERR_CALLBACK && close_err_cb );

void
close_sync();

void
close();
~~~~~

Async versions post callback on `asio::io_service` thread
and once executed one of specified callbacks is called.
The first one in case of success, and the second - in case of error.

Sync version work through async version getting synchronized by
future-promise.

`http_server_t<TRAITS>` also has methods to start/stop running
`asio::io_service`:
~~~~~
::c++
void
start_io_service();

void
stop_io_service();
~~~~~

They are helpful when using async versions of open()/close() methods,
when running `asio::io_service` and running server must be managed separately.

## Settings of *http_server_t*

Class `server_settings_t<TRAITS>` serves to pass settings to `http_server_t`.
It is defined in [restinio/settings.hpp](./dev/restinio/settings.hpp);

For each parameter a setter/getter pair is provided.
While setting most of parameters is pretty straightforward,
there are some parameters with a bit tricky setter/getter semantics.
They are request_handler, timer_factory, logger.

For example setter for request_handler looks like this:
~~~~~
::c++
template< typename... PARAMS >
server_settings_t &
request_handler( PARAMS &&... params );
~~~~~

When called an instance of `std::unique_ptr<TRAITS::request_handler_t>`
will be created with specified `params`.
If no constructor with such parameters is available,
then compilation error will occur.
If `request_handler_t` has a default constructor then it is not
mandatory to call setter -- the default constructed instance will be used.

As request handler is constructed as unique_ptr, then getter
returns unique_ptr value with ownership, so while manipulating
`server_settings_t` object don't use it.

The same applies to timer_factory and logger parameters.

When `http_server_t` instance is created request_handler, timer_factory and logger
are checked to be instantiated.

## Traits of *http_server_t*

### timer_factory_t
`timer_factory_t` - defines a timeout controller logic.
It must define a nested type `timer_guard_t` with the following interface:

~~~~~
::c++
class timer_guard_t
{
  public:
    // Set new timeout guard.
    template <
        typename EXECUTOR,
        typename CALLBACK_FUNC >
    void
    schedule_operation_timeout_callback(
      const EXECUTOR & e,
      std::chrono::steady_clock::duration d,
      CALLBACK_FUNC && cb );

    // Cancel timeout guard if any.
    void
    cancel();
};
~~~~~

The first method starts guarding timeout of a specified duration,
and if it occurs some how the specified callback must be posted on
`asio::io_service` executor.

The second method must cancel execution of the previously scheduled timer.

An instance of `std::shared_ptr< timer_guard_t >` is stored in each connection
managed by *RESTinio* and to create it `timer_factory_t` must define
the following method:

~~~~~
::c++
class timer_factory_t
{
  public:
    // ...

    using timer_guard_instance_t = std::shared_ptr< timer_guard_t >;

    // Create guard for connection.
    timer_guard_instance_t
    create_timer_guard( asio::io_service & );
    // ...
};
~~~~~

*RESTinio* comes with a set of ready-to-use `timer_factory_t` implementation:

* `null_timer_factory_t` -- noop timer guards, they produce timer guards
that do nothing (when no control needed).
See [restinio/null_timer_factory.hpp](./dev/restinio/null_timer_factory.hpp);
* `asio_timer_factory_t` -- timer guards implemented with asio timers.
See [restinio/asio_timer_factory.hpp](./dev/restinio/asio_timer_factory.hpp);
* `so5::so_timer_factory_t` -- timer guards implemented with *SObjectizer* timers.
See [restinio/so5/so_timer_factory.hpp](./dev/restinio/so5/so_timer_factory.hpp)
Note that `restinio/so5/so_timer_factory.hpp` header file is not included
by `restinio/all.hpp`, so it needs to be included separately.

### logger_t
`logger_t` - defines a logger implementation.
It must support the following interface:
~~~~~
::c++
class null_logger_t
{
  public:
    template< typename MSG_BUILDER >
    void trace( MSG_BUILDER && mb );

    template< typename MSG_BUILDER >
    void info( MSG_BUILDER && mb );

    template< typename MSG_BUILDER >
    void warn( MSG_BUILDER && mb );

    template< typename MSG_BUILDER >
    void error( MSG_BUILDER && mb );
};
~~~~~

`MSG_BUILDER` is lambda that returns a message to log out.
This approach allows compiler to optimize logging when it is possible,
see [`null_logger_t`](./dev/restinio/loggers.hpp).

For implementation example see
[`ostream_logger_t`](./dev/restinio/ostream_logger.hpp).

### request_handler_t

`request_handler_t` - is a key type for request handling process.
It must be a function-object with the following invocation interface:
~~~~~
::c++
restinio::request_handling_status_t
handler( restinio::request_handle_t req );
~~~~~

The `req` parameter defines request data and stores some data
necessary for creating responses.
Parameter is passed by value and thus can be passed to another
processing flow (that is where an async handling becomes possible).

Handler must return handling status via `request_handling_status_t` enum.
If handler handles request it must return `accepted`.

If handler refuses to handle request it must return `rejected`.

### strand_t

`strand_t` provides serialized callback invocation
for events of a specific connection.
There are two option for `strand_t`:
`asio::strand< asio::executor >` or `asio::executor`.

By default `asio::strand< asio::executor >` is used,
it guarantees serialized chain of callback invocation.
But if `asio::ioservice` runs on a single thread there is no need
to use `asio::strand` because there is no way to run callbacks
in parallel. So in such cases it is enough to use `asio::executor`
directly and eliminate overhead of `asio::strand`.

## Request handling

Lets consider that we are at the point when response
on a particular request is ready to be created and send.
The key here is to use a given connection handle and
[response_builder_t](./dev/restinio/message_builders.hpp):

Basic example of response builder with default response builder:
~~~~~
::c++
// Construct response builder.
auto resp = req->create_response(); // 200 OK

// Set header fields:
resp.append_header( "Server", "RESTinio server" );
resp.append_header_date_field();
resp.append_header( "Content-Type", "text/plain; charset=utf-8" );

// Set body:
resp.set_body( "Hello world!" );

// Response is ready, send it:
resp.done();
~~~~~

Currently there are three types of response builders.
Each builder type is a specialization of a template class `response_builder_t< TAG >`
with a specific tag-type:

* Tag `restinio_controlled_output_t`. Simple standard response builder.
* Tag `user_controlled_output_t`. User controlled response output builder.
* Tag `chunked_output_t`. Chunked transfer encoding output builder.

### Simple standard response builder

Requires user to set header and body.
Content length is automatically calculated.
Once the data is ready, the user calls done() method
and the resulting response is scheduled for sending.

~~~~~
::c++
 handler =
  []( auto req ) {
    if( restinio::http_method_get() == req->header().method() &&
      req->header().request_target() == "/" )
    {
      req->create_response()
        .append_header( "Server", "RESTinio hello world server" )
        .append_header_date_field()
        .append_header( "Content-Type", "text/plain; charset=utf-8" )
        .set_body( "Hello world!")
        .done();

      return restinio::request_accepted();
    }

    return restinio::request_rejected();
  };
~~~~~

### User controlled response output builder

This type of output allows user
to send body divided into parts.
But it is up to user to set the correct
Content-Length field.

~~~~~
::c++
 handler =
  []( restinio::request_handle_t req ){
    using output_type_t = restinio::user_controlled_output_t;
    auto resp = req->create_response< output_type_t >();

    resp.append_header( "Server", "RESTinio" )
      .append_header_date_field()
      .append_header( "Content-Type", "text/plain; charset=utf-8" )
      .set_content_length( req->body().size() );

    resp.flush(); // Send only header

    for( const char c : req->body() )
    {
      resp.append_body( std::string{ 1, c } );
      if( '\n' == c )
      {
        resp.flush();
      }
    }

    return resp.done();
  }
~~~~~

### Chunked transfer encoding output builder

This type of output sets transfer-encoding to chunked
and expects user to set body using chunks of data.

~~~~~
::c++
 handler =
  []( restinio::request_handle_t req ){
    using output_type_t = restinio::chunked_output_t;
    auto resp = req->create_response< output_type_t >();

    resp.append_header( "Server", "RESTinio" )
      .append_header_date_field()
      .append_header( "Content-Type", "text/plain; charset=utf-8" );

    resp.flush(); // Send only header


    for( const char c : req->body() )
    {
      resp.append_chunk( std::string{ 1, c } );
      if( '\n' == c )
      {
        resp.flush();
      }
    }

    return resp.done();
  }
~~~~~

## Routers

One of the reasons to create *RESTinio* was an ability to have
[express](https://expressjs.com/)-like request handler router.

Since v 0.2.1 *RESTinio* has a router based on idea borrowed
from [express](https://expressjs.com/) - a JavaScript framework.

Routers acts as a request handler (it means it is a function-object
that can be called as a request handler).
But router aggregates several handlers and picks one or none of them
to handle the request. The choice of the handler to execute depends on
request target and HTTP method. If router finds no handler matching request then it
rejects it.
Note that that the signature of the handlers put in router
are not the same as standard request handler.
It has an additional parameter -- a container with parameters extracted from URI.

Express router is defined by `express_router_t` class.
Its implementation is inspired by
[express-router](https://expressjs.com/en/starter/basic-routing.html).
It allows to define route path with injection
of parameters that become available for handlers.
For example the following code sets a handler with 2 parameters:
```
::c++
  router.http_get(
    R"(/article/:article_id/:page(\d+))",
    []( auto req, auto params ){
      const auto article_id = params[ "article_id" ];
      auto page = std::to_string( params[ "page" ] );
      // ...
    } );
```

Note that express handler receives 2 parameters not only request handle
but also `route_params_t` instance that holds parameters of the request:
```
::c++
using express_request_handler_t =
    std::function< request_handling_status_t( request_handle_t, route_params_t ) >;
```

Route path defines a set of named and indexed parameters.
Named parameters starts with `:`, followed by non-empty parameter name
(only A-Za-z0-9_ are allowed). After parameter name it is possible to
set a capture regex enclosed in brackets
(actually a subset of regex - none of the group types are allowed).
Indexed parameters are simply a capture regex in brackets.

Let's show how it works by example.
First let's assume that variable `router` is a pointer to express router.
So that is how we add a request handler with a single parameter:
```
::c++
  // GET request with single parameter.
  router->http_get( "/single/:param", []( auto req, auto params ){
    return
      init_resp( req->create_response() )
        .set_body( "GET request with single parameter: " + params[ "param" ] )
        .done();
  } );
```

The following requests will be routed to that handler:

* http://localhost/single/123 param="123"
* http://localhost/single/parameter/ param="parameter"
* http://localhost/single/another-param param="another-param"

But the following will not:

* http://localhost/single/123/and-more
* http://localhost/single/
* http://localhost/single-param/123

Let's use more parameters and assign a capture regex for them:
```
::c++
  // POST request with several parameters.
  router->http_post( R"(/many/:year(\d{4}).:month(\d{2}).:day(\d{2}))",
    []( auto req, auto params ){
      return
        init_resp( req->create_response() )
          .set_body( "POST request with many parameters:\n"
            "year: "+ params[ "year" ] + "\n" +
            "month: "+ params[ "month" ] + "\n" +
            "day: "+ params[ "day" ] + "\n"
            "body: " + req->body() )
          .done();
    } );
```
The following requests will be routed to that handler:

* http://localhost/many/2017.01.01 year="2017", month="01", day="01"
* http://localhost/many/2018.06.03 year="2018", month="06", day="03"
* http://localhost/many/2017.12.22 year="2017", month="12", day="22"

But the following will not:

* http://localhost/many/2o17.01.01
* http://localhost/many/2018.06.03/events
* http://localhost/many/17.12.22

Using indexed parameters is practically the same, just omit parameters names:
```
::c++
  // GET request with indexed parameters.
  router->http_get( R"(/indexed/([a-z]+)-(\d+)/(one|two|three))",
    []( auto req, auto params ){
      return
        init_resp( req->create_response() )
          .set_body( "POST request with indexed parameters:\n"
            "#0: "+ params[ 0 ] + "\n" +
            "#1: "+ params[ 1 ] + "\n" +
            "#2: "+ params[ 2 ] + "\n" )
          .done();
    } );
```
The following requests will be routed to that handler:

* http://localhost/indexed/xyz-007/one #0="xyz", #1="007", #2="one"
* http://localhost/indexed/ABCDE-2017/two #0="ABCDE", #1="2017", #2="two"
* http://localhost/indexed/sobjectizer-5/three #0="sobjectizer", #1="5", #2="three"

But the following will not:

* http://localhost/indexed/xyz-007/zero
* http://localhost/indexed/173-xyz/one
* http://localhost/indexed/ABCDE-2017/one/two/three

See full [example](./dev/sample/express_router_tutorial.cpp)

For details on `route_params_t` and `express_router_t` see
[express.hpp](./dev/restinio/router/express.cpp).

# Road Map

Features for next releases:

* Non trivial benchmarks. Comparison with other libraries with similar features
on the range of various scenarios.
* HTTP client. Introduce functionality for building and sending requests and
receiving and parsing results.
* TLS support.
* Full CMake support.
* Want more features?
Please send us feedback and we will take your opinion into account.

## Done features

*0.2.1*

* Routers for message handlers.
Support for a URI dependent routing to a set of handlers (express-like router).
* Enable `localhost` and `ip6-localhost` aliases for setting server address.

*0.2.0*

* [HTTP pipelining](https://en.wikipedia.org/wiki/HTTP_pipelining) support.
Read, parse and call a handler for incoming requests independently.
When responses become available send them to client in order of corresponding requests.
* Support for chunked transfer encoding. Separate responses on header and body chunks,
so it will be possible to send header and then body divided on chunks.

# License

*RESTinio* is distributed under GNU Affero GPL v.3 license (see [LICENSE](./LICENSE) and [AGPL](./agpl-3.0.txt) files).

For the license of *asio* library see COPYING file in *asio* distributive.

For the license of *nodejs/http-parser* library
see LICENSE file in *nodejs/http-parser* distributive.

For the license of *fmtlib* see LICENSE file in *fmtlib* distributive.

For the license of *SObjectizer* library see LICENSE file in *SObjectizer* distributive.

For the license of *rapidjson* library see LICENSE file in *rapidjson* distributive.

For the license of *json_dto* library see LICENSE file in *json_dto* distributive.

For the license of *args* library see LICENSE file in *args* distributive.

For the license of *CATCH* library see LICENSE file in *CATCH* distributive.
