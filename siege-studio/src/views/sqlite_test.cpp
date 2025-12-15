#include <winsqlite/winsqlite3ext.h>
SQLITE_EXTENSION_INIT1
#include <memory>
#include <string_view>
#include <span>
#include <cassert>
#include <siege/platform/win/basic_window.hpp>

struct vtable : sqlite3_vtab
{
  enum table_type
  {
    unknown,
    window,
    module,
    thread
  } table_type;

  std::vector<HANDLE> handles;
};

int SQLITE_CALLBACK Create(sqlite3* db, void* aux, int argc, const char* const* argv, sqlite3_vtab** vtab, char**)
{
  if (argc < 3)
  {
    return SQLITE_ERROR;
  }

  std::string_view name = argv[2];

  void* mem = sqlite3_api->malloc(sizeof(vtable));

  if (!mem)
  {
    return SQLITE_NOMEM;
  }

  int result = SQLITE_ERROR;
  if (name == "windows")
  {
    int result = sqlite3_api->declare_vtab(db, "CREATE TABLE windows(handle INTEGER PRIMARY KEY, name STRING, thread_id INTEGER) WITHOUT ROWID");

    if (result != SQLITE_OK)
    {
      sqlite3_api->free(mem);
      return result;
    }

    *vtab = new (mem) vtable{ .table_type = vtable::window };
  }
  else if (name == "modules")
  {
    result = sqlite3_api->declare_vtab(db, "CREATE TABLE modules(handle INTEGER PRIMARY KEY, name STRING) WITHOUT ROWID");

    if (result != SQLITE_OK)
    {
      sqlite3_api->free(mem);
      return result;
    }
    *vtab = new (mem) vtable{ .table_type = vtable::module };
  }
  else if (name == "threads")
  {
    result = sqlite3_api->declare_vtab(db, "CREATE TABLE threads(id INTEGER PRIMARY KEY) WITHOUT ROWID");

    if (result != SQLITE_OK)
    {
      sqlite3_api->free(mem);
      return result;
    }
    *vtab = new (mem) vtable{ .table_type = vtable::thread };
  }
  else
  {
    return result;
  }
  return SQLITE_OK;
}


int SQLITE_CALLBACK Destroy(sqlite3_vtab* vtab)
{
  vtable* real = static_cast<vtable*>(vtab);
  real->~vtable();
  sqlite3_api->free(vtab);
  return 0;
}


int SQLITE_CALLBACK Connect(sqlite3* db, void* pAux, int argc, const char* const* argv, sqlite3_vtab** vtab, char**)
{
  return SQLITE_OK;
}

int SQLITE_CALLBACK Disconnect(sqlite3_vtab* vtab)
{
  vtable* real = static_cast<vtable*>(vtab);
  real->~vtable();
  sqlite3_api->free(real);
  return SQLITE_OK;
}

struct cursor : sqlite3_vtab_cursor
{
  std::size_t size;
  std::vector<HANDLE>::iterator curr;
  std::vector<HANDLE>::iterator end;
};

int SQLITE_CALLBACK Open(sqlite3_vtab* pVTab, sqlite3_vtab_cursor** cur)
{
  void* mem = sqlite3_api->malloc(sizeof(cursor));

  if (!mem)
  {
    return SQLITE_NOMEM;
  }

  *cur = new (mem) cursor{};

  return 0;
}

int SQLITE_CALLBACK Close(sqlite3_vtab_cursor* cur)
{
  cursor* real = static_cast<cursor*>(cur);
  real->~cursor();
  sqlite3_api->free(cur);
  return 0;
}

int SQLITE_CALLBACK Next(sqlite3_vtab_cursor* cur)
{
  cursor* real = static_cast<cursor*>(cur);
  if (real->size == 0)
  {
    return 0;
  }
  std::advance(real->curr, 1);
  return 0;
}

int SQLITE_CALLBACK Eof(sqlite3_vtab_cursor* cur)
{
  cursor* real = static_cast<cursor*>(cur);

  if (real->size == 0)
  {
    return 1;
  }
  return real->curr == real->end;
}

int SQLITE_CALLBACK Rowid(sqlite3_vtab_cursor* cur, sqlite3_int64* row_id)
{
  return SQLITE_ERROR;
}

int SQLITE_CALLBACK Column(sqlite3_vtab_cursor* cur, sqlite3_context* ctx, int col)
{
  cursor* real = static_cast<cursor*>(cur);

  if (col == 0)
  {
    sqlite3_api->result_int64(ctx, (sqlite_int64)*real->curr);
  }
  else if (col == 1)
  {
    auto window = (HWND)*real->curr;
    auto count = ::GetWindowTextLengthW(window);
    auto* buffer = (wchar_t*)sqlite3_api->malloc(count * 2 + 2);
    if (!buffer)
    {
      return SQLITE_NOMEM;
    }

    sqlite3_api->result_text16(ctx, buffer, ::GetWindowTextW(window, buffer, count + 1) * 2, sqlite3_api->free);
  }
  else if (col == 2)
  {
    auto window = (HWND)*real->curr;
    auto thread_id = ::GetWindowThreadProcessId(window, nullptr);

    sqlite3_api->result_int(ctx, thread_id);
  }

  return SQLITE_OK;
}

int SQLITE_CALLBACK Filter(sqlite3_vtab_cursor* cur, int idxNum, const char* idxStr, int argc, sqlite3_value** argv)
{
  cursor* real = static_cast<cursor*>(cur);
  auto* table = static_cast<vtable*>(cur->pVtab);

  if (table->table_type == vtable::window && (idxNum == 1 || idxNum == 2))
  {
    if (real->size == 0)
    {
      table->handles.clear();
      struct callback
      {
        static BOOL __stdcall EnumThreadWndProc(HWND hwnd, LPARAM lParam)
        {
          auto* handles = (std::vector<HANDLE>*)lParam;
          handles->emplace_back(hwnd);
          ::EnumChildWindows(hwnd, EnumThreadWndProc, lParam);
          return TRUE;
        }
      };

      if (idxNum == 1)
      {
        ::EnumWindows(callback::EnumThreadWndProc, (LPARAM)&table->handles);
      }
      else
      {
        assert(argc > 0);
        auto thread_id = sqlite3_api->value_int(argv[0]);
        ::EnumThreadWindows(thread_id, callback::EnumThreadWndProc, (LPARAM)&table->handles);
      }
    }

    real->size = table->handles.size();
    real->curr = table->handles.begin();
    real->end = table->handles.end();
  }


  return SQLITE_OK;
}

int SQLITE_CALLBACK BestIndex(sqlite3_vtab* vtab, sqlite3_index_info* info)
{
  auto* table = static_cast<vtable*>(vtab);
  
  std::span<const sqlite3_index_info::sqlite3_index_constraint> constraints(info->aConstraint, info->nConstraint);

  if (table->table_type == vtable::window)
  {
    auto thread_id_iter = std::find_if(constraints.begin(), constraints.end(), [](auto& item) {
      return item.iColumn == 2;
    });

    if (thread_id_iter != constraints.end())
    {
      auto index = std::distance(constraints.begin(), thread_id_iter);
      info->aConstraintUsage[index].argvIndex = 1;
      info->aConstraintUsage[index].omit = 1;
      info->idxNum = 2;
      info->estimatedCost = 5.0;
    }
    else
    {
      info->idxNum = 1;
      info->estimatedCost = 10.0;
    }
  }
  else
  {
    info->estimatedCost = 10.0;
  }

  return SQLITE_OK;
}

int SQLITE_CALLBACK Update(sqlite3_vtab*, int, sqlite3_value**, sqlite3_int64*)
{
  return 0;
}

int SQLITE_CALLBACK Begin(sqlite3_vtab* pVTab)
{
  return 0;
}

int SQLITE_CALLBACK Sync(sqlite3_vtab* pVTab)
{
  return 0;
}

int SQLITE_CALLBACK Commit(sqlite3_vtab* pVTab)
{
  return 0;
}

int SQLITE_CALLBACK Rollback(sqlite3_vtab* pVTab)
{
  return 0;
}

int SQLITE_CALLBACK FindFunction(sqlite3_vtab* pVtab, int nArg, const char* zName, void(SQLITE_CALLBACK** pxFunc)(sqlite3_context*, int, sqlite3_value**), void** ppArg)
{
  return 0;
}

int SQLITE_CALLBACK Rename(sqlite3_vtab* pVtab, const char* zNew)
{
  return 0;
}

int SQLITE_CALLBACK test_vtab_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* api)
{
  static sqlite3_module module = {
    .xCreate = Create,
    .xConnect = Create,
    .xBestIndex = BestIndex,
    .xDisconnect = Destroy,
    .xDestroy = Destroy,
    .xOpen = Open,
    .xClose = Close,
    .xFilter = Filter,
    .xNext = Next,
    .xEof = Eof,
    .xColumn = Column,
    //   .xRowid = Rowid
  };

  SQLITE_EXTENSION_INIT2(api);
  auto result = sqlite3_api->create_module(db, "magic_module", &module, 0);
  return result;
}


/*
int iVersion;
  /* The methods above are in version 1 of the sqlite_module object. Those
  ** below are for version 2 and greater. */
// int(SQLITE_CALLBACK* xSavepoint)(sqlite3_vtab* pVTab, int);
// int(SQLITE_CALLBACK* xRelease)(sqlite3_vtab* pVTab, int);
// int(SQLITE_CALLBACK* xRollbackTo)(sqlite3_vtab* pVTab, int);
///* The methods above are in versions 1 and 2 of the sqlite_module object.
//** Those below are for version 3 and greater. */
// #if NTDDI_VERSION >= NTDDI_WIN10_VB
// int(SQLITE_CALLBACK* xShadowName)(const char*);
//*/