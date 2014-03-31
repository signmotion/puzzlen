#pragma once

#include <assert.h>
#include <string>


namespace puzzlen {


// Путь к изображениям.
static const std::wstring  PATH_MEDIA = L"../media";




// Размер поля по умолчанию.
static const size_t DEFAULT_N = 4;
static const size_t DEFAULT_M = DEFAULT_N;




// Расстояние, при котором смещаемый элемент сам "прилипнет" к соседу, %.
static const size_t GLUE_PERCENT = 20;




// Размер ячейки в пкс.
static const size_t CELL_SIZE = 50;




// Для отладки.
#ifdef _DEBUG
#define ASSERT(EXPR)   assert(EXPR);
#define DASSERT(EXPR)  if (!(EXPR)) __debugbreak();

#define QUOTE_(WHAT)      #WHAT
#define QUOTE(WHAT)       QUOTE_(WHAT)
#define DBG(format, ...)  printf("%s: "format, __FILE__":"QUOTE(__LINE__), ## __VA_ARGS__)

// Состояние PuzzleN отразится в заголовке окна.
//#define CONSOLE_DEBUG_PUZZLEN

#else
#define ASSERT(EXPR)      ((void)0)
#define DASSERT(EXPR)     ((void)0)
#define DBG(format, ...)  ((void)0)
#define CONSOLE           ((void)0)

#endif


} // puzzlen
