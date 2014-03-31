#pragma once

// Отключим макрос в WinDef.h
#define NOMINMAX

// настройка для GDI+
#undef WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <algorithm>
using std::min;
using std::max;
#include <GdiPlus.h>

#include <assert.h>
#include <iostream>
#include <math.h>
#include <memory>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
#include <windowsx.h>

#include "Exception.h"
