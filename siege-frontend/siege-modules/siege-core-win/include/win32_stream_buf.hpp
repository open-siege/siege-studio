#ifndef WIN32_STREAM_BUF_HPP
#define WIN32_STREAM_BUF_HPP

#include <istream>
#include <memory>
#include <objidl.h>

namespace win32::com
{
    template<typename IStreamContainer = std::unique_ptr<IStream, void(*)(IStream*)>>
    class IStreamBuf : std::streambuf
    {
        
    private:
        IStreamContainer data;
        std::array<char, 16> read_buffer;
        std::array<char, 16> write_buffer;
    public:
        IStreamBuf(IStreamContainer data) : data(data)
        {
            setg(read_buffer.data(), read_buffer.data() + read_buffer.size(), read_buffer.data() + read_buffer.size());
            setp(write_buffer.data() + write_buffer.size(), write_buffer.data() + write_buffer.size());
        }

    protected:
        std::streambuf* setbuf(char_type* s, std::streamsize n) override final
        {
            return this;
        }

        pos_type seekpos( pos_type pos,
                          std::ios_base::openmode which = std::ios_base::in | std::ios_base::out ) override final
        {
            ULARGE_INTEGER result{};
            LARGE_INTEGER largePos{};
            largePos.QuadPart = pos;

            if (data.Seek(largePos, STREAM_SEEK_SET, &result) == S_OK)
            {
                return pos_type(result.QuadPart);

            }
            return 0;
        }

        pos_type seekoff( off_type off, std::ios_base::seekdir dir,
                          std::ios_base::openmode which = std::ios_base::in | std::ios_base::out ) override final
        {
            ULARGE_INTEGER result{};
            LARGE_INTEGER largePos{};
            largePos.QuadPart = off;

            DWORD direction = STREAM_SEEK_CUR;

            if (dir == std::ios_base::beg)
            {
                direction = STREAM_SEEK_SET;
            }
            else if (dir == std::ios_base::end)
            {
                direction = STREAM_SEEK_END;
            }

            if (data.Seek(largePos, direction, &result) == S_OK)
            {
                return pos_type(result.QuadPart);

            }
            return 0;
        }

        int sync() override final
        {
            return data.Commit(STGC_DEFAULT) == S_OK ? 0 : 1;
        }

        std::streamsize showmanyc() override final
        {
            STATSTG result{};

            if (data.Stat(&result, STATFLAG_NOOPEN) == S_OK)
            {
                // TODO is the return value the entire size or the size from the current stream position.
                return std::streamsize(result.cbSize.QuadPart);
            }
                
            return 0;        
        }

        std::streamsize xsgetn( char_type* s, std::streamsize count ) override final
        {
            ULONG result = 0;

            auto hresult = data.Read(s, count, &result);
            
            if (hresult == S_OK || hresult == S_FALSE)
            {
                return result;
            }

            return 0;
        }

        std::streambuf::int_type underflow() override final
        {
            ULONG result = 0;

            auto hresult = data.Read(read_buffer.data(), ULONG(read_buffer.size()), &result);
            
            if (hresult == S_OK || hresult == S_FALSE)
            {
                setg(read_buffer.data(), read_buffer.data(), read_buffer.data() + read_buffer.size());
                return hresult == S_FALSE ? traits_type::eof() : int_type(read_buffer[0]);
            }

            return traits_type::eof();
        }

        std::streamsize xsputn(const char_type* s, std::streamsize count) override final
        {
            ULONG result = 0;

            auto hresult = data.Write(s, count, &result);
            
            if (hresult == S_OK)
            {
                return result;
            }

            return 0;
        }

        std::streambuf::int_type overflow(std::streambuf::int_type charater) override final
        {
            ULONG result = 0;

            auto hresult = data.Write(write_buffer.data(), write_buffer.size(), &result);
            
            if (hresult == S_OK)
            {
                return result;
            }

            return traits_type::eof();
        }
    };

}

#endif