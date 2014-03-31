/**
* Заготовка для игры "пятнашки".
*
* Запускается приложение из консоли командой "puzzlen [N [M]]".
* Где N, M - количество ячеек по ширине и высоте.
* Пример: puzzlen 7 10
*
* Управление
*   LeftClick + move  Перемещает элемент.
*   SPACE             Перетасовывает элементы.
*   ESC               Выход.
*
* @see configure.h для установки параметров.
*
* @author Андрей Сырокомский, +38 050 335-16-18
*/


#include "include/stdafx.h"
#include "include/PuzzleN.h"


static std::unique_ptr< puzzlen::PuzzleN >  puzzlenPtr;


// Распознаёт параметры приложения.
std::pair< size_t, size_t >  parse( const LPSTR cmdLine );


// Для визуальной отладки.
void debug( HWND wnd );




// Исп. для работы с GDI+.
LRESULT CALLBACK wndProc(
    HWND wnd, UINT message, WPARAM wparam, LPARAM lparam
);

void draw( HDC hdc,  const RECT& rc );




int WINAPI WinMain(
    HINSTANCE instance,
    HINSTANCE prevInstance,
    LPSTR cmdLine,
    int cmdShow
) {
    using namespace puzzlen;
    using namespace Gdiplus;


    setlocale( LC_ALL, "Russian" );
    setlocale( LC_NUMERIC, "C" );


    // инициализируем GDI+
    GdiplusStartupInput  gdiplusStartupInput; 
    ULONG_PTR  gdiplusToken; 
    GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, nullptr );


    // создаём окно визуализации
    WNDCLASSEX ex;
    ex.cbSize        = sizeof( WNDCLASSEX );
    ex.style         = 0;
    ex.lpfnWndProc   = wndProc;
    ex.cbClsExtra    = 0;
    ex.cbWndExtra    = 0;
    ex.hInstance     = instance;
    ex.hIcon         = LoadIcon( instance, MAKEINTRESOURCE( IDI_APPLICATION ) );
    ex.hCursor       = LoadCursor( nullptr, IDC_UPARROW );
    ex.hbrBackground = (HBRUSH) GetStockObject( WHITE_BRUSH );
    ex.lpszMenuName  = nullptr;
    ex.lpszClassName = "win32app";
    ex.hIconSm = LoadIcon( ex.hInstance, MAKEINTRESOURCE( IDI_APPLICATION ) );

    if ( !RegisterClassEx( &ex ) ) {
        MessageBox( nullptr, "(!) Register class failed.", "PuzzleN", 0 );
        return -1;
    }


    std::pair< size_t, size_t >  params;
    try {
        params = parse( cmdLine );

    } catch ( const puzzlen::Exception& ex ) {
        MessageBox( nullptr, ex.what(), "PuzzleN", 0 );
        return -1;
    }


    const int WINDOW_WIDTH  = params.first  * CELL_SIZE;
    const int WINDOW_HEIGHT = params.second * CELL_SIZE;

    auto wnd = CreateWindow(
        "win32app", "",
        WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr,
        nullptr,
        instance,
        nullptr
    );
    if ( !wnd )  {
        MessageBox( nullptr, "(!) Create window failed.", "PuzzleN", 0 );
        return -1;
    }

    // центрируем окно, корректируем размер
    {
        RECT  rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
        const DWORD style = GetWindowLongPtr( wnd, GWL_STYLE );
        const HMENU menu = GetMenu( wnd );
        AdjustWindowRect( &rc, style, menu ? true : false );
        const int x = (GetSystemMetrics( SM_CXSCREEN ) - rc.right)  / 2;
        const int y = (GetSystemMetrics( SM_CYSCREEN ) - rc.bottom) / 3;
        SetWindowPos(
            wnd,
            HWND_TOPMOST,
            x, y,
            rc.right  - rc.left,
            rc.bottom - rc.top,
            SWP_NOZORDER
        );
    }


    // создаём PuzzleN
    try {
        puzzlenPtr = std::unique_ptr< PuzzleN >(
            new PuzzleN( params.first, params.second, CELL_SIZE )
        );
    } catch ( const Exception& ex ) {
        std::cerr << ex.what() << std::endl;
        return -1;
    }


    // заголовок окна
    std::ostringstream  title;
    title << "Puzzle  " << puzzlenPtr->N << " x " << puzzlenPtr->M;
    SetWindowText( wnd,  title.str().c_str() );


    // визуализируем
    ShowWindow( wnd, cmdShow );
    UpdateWindow( wnd );




    MSG msg;
    while ( GetMessage( &msg, nullptr, 0, 0 ) ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    GdiplusShutdown( gdiplusToken );

    return 0;
}








LRESULT CALLBACK wndProc(
    HWND wnd, UINT message, WPARAM wparam, LPARAM lparam
) {
    PAINTSTRUCT ps;
    HDC hdc;

    switch ( message ) {
        case WM_CREATE:
            SetTimer( wnd, 0, 10, 0 );
            return 0;

        case WM_TIMER:
            InvalidateRect( wnd, nullptr, false );
            UpdateWindow( wnd );
#ifdef CONSOLE_DEBUG_PUZZLEN
            debug( wnd );
#endif
            return 0;

        case WM_PAINT:
            hdc = BeginPaint( wnd, &ps );
            puzzlenPtr->draw( hdc, ps.rcPaint );
            EndPaint( wnd, &ps );
            return 0;

        case WM_LBUTTONDOWN:
            puzzlenPtr->pressMouseButton( true );
            puzzlenPtr->firstClick(
                GET_X_LPARAM( lparam ),
                GET_Y_LPARAM( lparam )
            );
            break;

        case WM_MOUSEMOVE:
            puzzlenPtr->move(
                GET_X_LPARAM( lparam ),
                GET_Y_LPARAM( lparam )
            );
            break;

        case WM_LBUTTONUP:
            puzzlenPtr->pressMouseButton( false );
            puzzlenPtr->stickMove();
            puzzlenPtr->resetFirstClick();
            break;

        case WM_KEYUP:
            if (wparam == VK_SPACE) {
                puzzlenPtr->shuffle();
            } else if (wparam == VK_ESCAPE) {
                PostQuitMessage( 0 );
            }
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( wnd, message, wparam, lparam);
}




void
debug( HWND wnd ) {

    std::ostringstream  ss;
    const auto& am = puzzlenPtr->aboutMove();
    ss << "i(" << am.i << ")  " <<
        "  " << am.firstClick.x << " " << am.firstClick.y <<
        "  " << am.shift.x << " " << am.shift.y <<
        "  |" << am.emptyClickShift.x << " " << am.emptyClickShift.y << "|"
        "  " << (puzzlenPtr->pressMouseButton() ? "true" : "false");
    SetWindowText( wnd,  ss.str().c_str() );
}








std::pair< size_t, size_t >
parse( const LPSTR cmdLine ) {

    using namespace puzzlen;

    size_t n = DEFAULT_N;
    size_t m = DEFAULT_M;
    std::istringstream  ss( cmdLine, std::istringstream::in );
    std::string  word;
    size_t count = 0;
    for ( ; ss >> word; ++count) {
        std::istringstream  wss( word );
        switch ( count ) {
            // ширина
            case 0:
                wss >> n;
                if ( wss.fail() ) {
                    throw Exception( "Width of puzzle is not recognized." );
                }
                if ( (n > 10) || (n < 3) ) {
                    throw Exception( "Width of puzzle must have diapason [3; 10]." );
                }
                break;

            // высота
            case 1:
                wss >> m;
                if ( wss.fail() ) {
                    throw Exception( "Height of puzzle is not recognized." );
                }
                if ( (m > 10) || (m < 3) ) {
                    throw Exception( "Height must have diapason [3; 10]." );
                }
                break;

            default:
                throw Exception( "Too many parameters in command line." );
        };
    };


    if (count == 1) {
        // # Допустимо не указывать высоту.
        m = n;
    }


    return std::make_pair( n, m );
}
