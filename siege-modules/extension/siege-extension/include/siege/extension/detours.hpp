#pragma once

// Extension-side Detours entry point. Pulls the lean Windows wrapper first so
// call sites do not include <Windows.h> or <detours.h> separately.
#include <siege/platform/windows.hpp>
#include <detours.h>
