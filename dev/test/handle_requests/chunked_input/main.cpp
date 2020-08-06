/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

RESTINIO_NODISCARD
restinio::fmt_minimal_memory_buffer_t
format_chunked_input_info(
	const restinio::request_t & req )
{
	restinio::fmt_minimal_memory_buffer_t resp_body;

	const auto * chunked_input = req.chunked_input_info();
	if( !chunked_input )
		fmt::format_to( resp_body, "no chunked input" );
	else
	{
		fmt::format_to( resp_body, "chunks:{};", chunked_input->chunk_count() );
		for( const auto & ch : chunked_input->chunks() )
			fmt::format_to( resp_body, "[{},{}]",
					ch.started_at(), ch.size() );
		fmt::format_to( resp_body, ";trailing_fields:{};",
				chunked_input->trailing_fields().fields_count() );
		for( const auto & f : chunked_input->trailing_fields() )
			fmt::format_to( resp_body, "('{}':'{}')", f.name(), f.value() );
	}

	return resp_body;
}

TEST_CASE( "Simple incoming request" , "[chunked-input][simple]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto req ){
						if( restinio::http_method_post() == req->header().method() )
						{
							restinio::fmt_minimal_memory_buffer_t resp_body =
								format_chunked_input_info( *req );

							req->create_response()
								.append_header( "Server", "RESTinio utest server" )
								.append_header_date_field()
								.append_header( "Content-Type", "text/plain; charset=utf-8" )
								.set_body( std::move(resp_body) )
								.done();
							return restinio::request_accepted();
						}

						return restinio::request_rejected();
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	SECTION( "no chunked-encoding" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 20\r\n"
			"Connection: close\r\n"
			"\r\n"
			"01234567890123456789"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"no chunked input") );
	}

	SECTION( "three chunks" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6\r\n"
			"Hello,\r\n"
			"1\r\n"
			" \r\n"
			"6\r\n"
			"World!\r\n"
			"0\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"chunks:3;[0,6][6,1][7,6];trailing_fields:0;") );
	}

	SECTION( "three chunks with trailing headers" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6\r\n"
			"Hello,\r\n"
			"1\r\n"
			" \r\n"
			"6\r\n"
			"World!\r\n"
			"0\r\n"
			"Header-1: Value-1\r\n"
			"Header-2: Value-2\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"chunks:3;[0,6][6,1][7,6];trailing_fields:2;"
						"('Header-1':'Value-1')('Header-2':'Value-2')" ) );
	}

	SECTION( "three chunks with trailing field without value" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6\r\n"
			"Hello,\r\n"
			"1\r\n"
			" \r\n"
			"6\r\n"
			"World!\r\n"
			"0\r\n"
			"Header-1:\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"chunks:3;[0,6][6,1][7,6];trailing_fields:1;"
						"('Header-1':'')" ) );
	}

	SECTION( "two chunks with chunk-ext" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6;a=b;c;d=e\r\n"
			"Hello,\r\n"
			"6;b=d;c=e\r\n"
			"World!\r\n"
			"0\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"chunks:2;[0,6][6,6];trailing_fields:0;") );
	}

	other_thread.stop_and_join();
}
