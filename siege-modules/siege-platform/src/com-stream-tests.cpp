#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <siege/platform/win/core/com/stream.hpp>

TEST_CASE("When StdStreamRef.Read has null parameters, the error code is invalid pointer.", "[win.core.com.stream]")
{
	std::stringstream storage;
	win32::com::StdStreamRef test(storage);
	REQUIRE(test.Read(nullptr, 0, nullptr) == STG_E_INVALIDPOINTER);
}

TEST_CASE("When StdStreamRef.Read points to empty storage, the byte written count is zero.", "[win.core.com.stream]")
{
	std::stringstream storage;
	win32::com::StdStreamRef test(storage);
	std::array<std::byte, 32> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), buffer.size(), &read);

	REQUIRE(read == 0);
}

TEST_CASE("When StdStreamRef.Read points to empty storage, the return code is false.", "[win.core.com.stream]")
{
	std::stringstream storage;
	win32::com::StdStreamRef test(storage);
	std::array<std::byte, 32> buffer{};
	ULONG read = -1;

	REQUIRE(test.Read(buffer.data(), buffer.size(), &read) == S_FALSE);
}

TEST_CASE("When StdStreamRef.Read has an empty buffer, the byte written count is zero.", "[win.core.com.stream]")
{
	std::stringstream storage;
	win32::com::StdStreamRef test(storage);
	std::array<std::byte, 1> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), 0, &read);
	REQUIRE(read == 0);
}

TEST_CASE("When StdStreamRef.Read has an empty buffer, the return code is ok.", "[win.core.com.stream]")
{
	std::stringstream storage;
	win32::com::StdStreamRef test(storage);
	std::array<std::byte, 1> buffer{};
	ULONG read = -1;

	REQUIRE(test.Read(buffer.data(), 0, &read) == S_OK);
}


TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and all bytes are requests, the byte written matches the total size.", "[win.core.com.stream]")
{
	std::stringstream storage("Hello World:");
	win32::com::StdStreamRef test(storage);
	std::array<std::byte, 12> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), buffer.size(), &read);
	REQUIRE(read == buffer.size());
}

TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and all bytes are requested, the return code is ok.", "[win.core.com.stream]")
{
	std::stringstream storage("Hello World:");
	win32::com::StdStreamRef test(storage);
	std::array<std::byte, 12> buffer{};
	ULONG read = -1;

	REQUIRE(test.Read(buffer.data(), buffer.size(), &read) == S_OK);
}

TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and all bytes are requested, the value read matches the original value.", "[win.core.com.stream]")
{
	std::stringstream storage("Hello World:");
	win32::com::StdStreamRef test(storage);
	std::array<std::byte, 12> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), buffer.size(), &read);
	REQUIRE(std::memcmp(buffer.data(), "Hello World:", 12) == 0);
}

TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and 5 bytes are requested, the bytes read matches the requested size.", "[win.core.com.stream]")
{
	std::stringstream storage("Hello World:");
	win32::com::StdStreamRef test(storage);
	std::array<std::byte, 5> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), buffer.size(), &read);
	REQUIRE(read == buffer.size());
}

TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and 5 bytes are requested, the return code is ok.", "[win.core.com.stream]")
{
	std::stringstream storage("Hello World:");
	win32::com::StdStreamRef test(storage);
	std::array<std::byte, 5> buffer{};
	ULONG read = -1;

	REQUIRE(test.Read(buffer.data(), buffer.size(), &read) == S_OK);
}

TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and 5 bytes are requested, the value read matches the original value.", "[win.core.com.stream]")
{
	std::stringstream storage("Hello World:");
	win32::com::StdStreamRef test(storage);
	std::array<std::byte, 5> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), buffer.size(), &read);
	REQUIRE(std::memcmp(buffer.data(), "Hello", 5) == 0);
}