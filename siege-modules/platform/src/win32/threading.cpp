module;
#include <siege/platform/windows.hpp>
export module win32.threading;

import std;

export namespace win32
{
  struct critical_section_leaver
  {
    void operator()(CRITICAL_SECTION* section)
    {
      if (section)
      {
        ::LeaveCriticalSection(section);
      }
    }
  };

  class critical_section
  {
  public:
    critical_section() : section{}
    {
      ::InitializeCriticalSection(&section);
    }

    std::unique_ptr<CRITICAL_SECTION, critical_section_leaver> enter()
    {
      ::EnterCriticalSection(&section);
      return std::unique_ptr<CRITICAL_SECTION, critical_section_leaver>(&section);
    }

    std::unique_ptr<CRITICAL_SECTION, critical_section_leaver> try_enter()
    {
      if (::TryEnterCriticalSection(&section))
      {
        return std::unique_ptr<CRITICAL_SECTION, critical_section_leaver>(&section);
      }

      return std::unique_ptr<CRITICAL_SECTION, critical_section_leaver>(nullptr);
    }

    ~critical_section()
    {
      ::DeleteCriticalSection(&section);
    }

    critical_section(const critical_section&) = delete;
    critical_section& operator=(const critical_section&) = delete;

    critical_section(critical_section&&) = default;
    critical_section& operator=(critical_section&&) = default;

  private:
    CRITICAL_SECTION section;
  };

  bool queue_user_work_item(std::move_only_function<void()> callback, ULONG flags = WT_EXECUTEDEFAULT)
  {
    struct handler
    {
      static DWORD WINAPI proc(LPVOID lpParameter)
      {
        auto func = (std::move_only_function<void()>*)lpParameter;
        try
        {
          if (func && *func)
          {
            func->operator()();
            delete func;
          }
        }
        catch (...)
        {
          if (func)
          {
            delete func;
          }
        }
        return 0;
      }
    };

    return QueueUserWorkItem(handler::proc, new std::move_only_function<void()>(std::move(callback)), flags);
  }
}// namespace win32

