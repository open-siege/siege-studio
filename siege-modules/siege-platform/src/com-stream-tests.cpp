#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <siege/platform/win/core/com/stream.hpp>

TEST_CASE("When StdIOStreamRef.QueryInterface has IUnknown, return code is okay.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);

	win32::com::com_ptr<IUnknown> temp;

	REQUIRE(test.QueryInterface(IID_IUnknown, temp.put_void()) == S_OK);
}

TEST_CASE("When StdIOStreamRef.QueryInterface has IUnknown, pointer is valid.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);

	win32::com::com_ptr<IUnknown> temp;

	REQUIRE(temp.get() == nullptr);
	test.QueryInterface(IID_IUnknown, temp.put_void());
	REQUIRE(temp.get() != nullptr);
}

TEST_CASE("When StdIOStreamRef.QueryInterface has ISequentialStream, return code is okay.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);

	win32::com::com_ptr<ISequentialStream> temp;

	REQUIRE(test.QueryInterface(IID_ISequentialStream, temp.put_void()) == S_OK);
}

TEST_CASE("When StdStreamRef.QueryInterface has ISequentialStream, pointer is valid.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);

	win32::com::com_ptr<ISequentialStream> temp;

	REQUIRE(temp.get() == nullptr);
	test.QueryInterface(IID_ISequentialStream, temp.put_void());
	REQUIRE(temp.get() != nullptr);
}

TEST_CASE("When StdStreamRef.QueryInterface has IStream, return code is okay.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);

	win32::com::com_ptr<IStream> temp;

	REQUIRE(test.QueryInterface(IID_IStream, temp.put_void()) == S_OK);
}

TEST_CASE("When StdStreamRef.QueryInterface has IStream, pointer is valid.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);

	win32::com::com_ptr<IStream> temp;

	REQUIRE(temp.get() == nullptr);
	test.QueryInterface(IID_IStream, temp.put_void());
	REQUIRE(temp.get() != nullptr);
}

TEST_CASE("When StdStreamRef.QueryInterface has IDispatch, return code is no interface.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);

	win32::com::com_ptr<IDispatch> temp;

	REQUIRE(test.QueryInterface(IID_IDispatch, temp.put_void()) == E_NOINTERFACE);
}

TEST_CASE("When StdStreamRef.Read has null parameters, the return code is invalid pointer.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	REQUIRE(test.Read(nullptr, 0, nullptr) == STG_E_INVALIDPOINTER);
}

TEST_CASE("When StdStreamRef.Read points to empty storage, the byte written count is zero.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 32> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), buffer.size(), &read);

	REQUIRE(read == 0);
}

TEST_CASE("When StdStreamRef.Read points to empty storage, the return code is false.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 32> buffer{};
	ULONG read = -1;

	REQUIRE(test.Read(buffer.data(), buffer.size(), &read) == S_FALSE);
}

TEST_CASE("When StdStreamRef.Read has an empty buffer, the byte written count is zero.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 1> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), 0, &read);
	REQUIRE(read == 0);
}

TEST_CASE("When StdStreamRef.Read has an empty buffer, the return code is ok.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 1> buffer{};
	ULONG read = -1;

	REQUIRE(test.Read(buffer.data(), 0, &read) == S_OK);
}


TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and all bytes are requests, the byte written matches the total size.")
{
	std::stringstream storage("Hello World:");
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 12> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), buffer.size(), &read);
	REQUIRE(read == buffer.size());
}

TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and all bytes are requested, the return code is ok.")
{
	std::stringstream storage("Hello World:");
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 12> buffer{};
	ULONG read = -1;

	REQUIRE(test.Read(buffer.data(), buffer.size(), &read) == S_OK);
}

TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and all bytes are requested, the value read matches the original value.")
{
	std::stringstream storage("Hello World:");
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 12> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), buffer.size(), &read);
	REQUIRE(std::memcmp(buffer.data(), "Hello World:", 12) == 0);
}

TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and 5 bytes are requested, the bytes read matches the requested size.")
{
	std::stringstream storage("Hello World:");
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 5> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), buffer.size(), &read);
	REQUIRE(read == buffer.size());
}

TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and 5 bytes are requested, the return code is ok.")
{
	std::stringstream storage("Hello World:");
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 5> buffer{};
	ULONG read = -1;

	REQUIRE(test.Read(buffer.data(), buffer.size(), &read) == S_OK);
}

TEST_CASE("When StdStreamRef.Read has 12 bytes in its storage and 5 bytes are requested, the value read matches the original value.")
{
	std::stringstream storage("Hello World:");
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 5> buffer{};
	ULONG read = -1;

	test.Read(buffer.data(), buffer.size(), &read);
	REQUIRE(std::memcmp(buffer.data(), "Hello", 5) == 0);
}

TEST_CASE("When StdStreamRef.Write has null parameters, the return code is invalid pointer.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	REQUIRE(test.Write(nullptr, 0, nullptr) == STG_E_INVALIDPOINTER);
}

TEST_CASE("When StdStreamRef.Write has an empty buffer, the return code is okay.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 1> buffer{};

	REQUIRE(test.Write(buffer.data(), 0, nullptr) == S_OK);
}

TEST_CASE("When StdStreamRef.Write has an empty buffer, with a counter, the return code is okay.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 1> buffer{};
	ULONG data_written = -1;

	REQUIRE(test.Write(buffer.data(), 0, &data_written) == S_OK);
}

TEST_CASE("When StdStreamRef.Write has an empty buffer, with a counter, the counter is zero.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 1> buffer{};
	ULONG data_written = -1;

	test.Write(buffer.data(), 0, &data_written);
	REQUIRE(data_written == 0);
}

TEST_CASE("When StdStreamRef.Write has a 16 byte buffer, with a null counter, the return code is okay.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 16> buffer{};
	std::memcpy(buffer.data(), "Hello world again", buffer.size());

	REQUIRE(test.Write(buffer.data(), buffer.size(), nullptr) == S_OK);
}

TEST_CASE("When StdStreamRef.Write has a 16 byte buffer, with a null counter, the written data is correct.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 16> buffer{};
	std::memcpy(buffer.data(), "Hello world again", buffer.size());

	test.Write(buffer.data(), buffer.size(), nullptr);

	REQUIRE(storage.str() == "Hello world agai");
}

TEST_CASE("When StdStreamRef.Write has a 16 byte buffer, with a counter, the return code is okay.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 16> buffer{};
	std::memcpy(buffer.data(), "Hello world again", buffer.size());
	ULONG data_written = -1;

	REQUIRE(test.Write(buffer.data(), buffer.size(), &data_written) == S_OK);
}

TEST_CASE("When StdStreamRef.Write has a 16 byte buffer, with a counter, the count is correct.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 16> buffer{};
	std::memcpy(buffer.data(), "Hello world again", buffer.size());
	ULONG data_written = -1;

	test.Write(buffer.data(), buffer.size(), &data_written);

	REQUIRE(data_written == buffer.size());
}

TEST_CASE("When StdStreamRef.Write has a 16 byte buffer, with a counter, the written data is correct.")
{
	std::stringstream storage;
	win32::com::StdIOStreamRef test(storage);
	std::array<std::byte, 16> buffer{};
	std::memcpy(buffer.data(), "Hello world again", buffer.size());
	ULONG data_written = -1;

	test.Write(buffer.data(), buffer.size(), &data_written);

	REQUIRE(storage.str() == "Hello world agai");
}


TEST_CASE("When StdStreamRef.Seek with a bad origin, the return code is invalid function.")
{
	std::stringstream storage("Hello world");
	win32::com::StdIOStreamRef test(storage);

	REQUIRE(test.Seek(LARGE_INTEGER{ .QuadPart = 10 }, 55, nullptr) == STG_E_INVALIDFUNCTION);
}