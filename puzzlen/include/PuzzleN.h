#pragma once

#include "configure.h"


namespace puzzlen {


class PuzzleN {
public:
    // Структуры для отработки перетаскивания элемента мышью.
    typedef struct {
        int x;
        int y;
    } coord_t;


    typedef coord_t  logicCoord_t;
    typedef coord_t  visualCoord_t;


    typedef struct {
        // какой элемент перемещается, индекс в 'field_t'
        int  i;
        // координаты первого клика мышью, пкс
        visualCoord_t  firstClick;
        // на сколько перемещается, пкс
        visualCoord_t  shift;
        visualCoord_t  emptyClickShift;
    } move_t;


    // В каких направления элемент может перемещаться.
    typedef struct {
        bool  north;
        bool  south;
        bool  west;
        bool  east;
    } permitShift_t;


    typedef size_t  element_t;
    // # Элементы храним в 1D-матрице.
    // # Индекс матрицы определяет позицию элемента на поле.
    // # Смещение 1 элемента храним в отдельной структуре - см. move_t.
    typedef std::vector< element_t >  field_t;


public:
    const size_t  N;
    const size_t  M;
    const size_t  cellSize;

    // Расстояние, при котором смещаемый элемент сам "прилипнет" к соседу.
    const size_t  glueDistance;


public:
    // @param cellSize  Размер ячейки для визуализации, пкс.
    PuzzleN( size_t n, size_t m, size_t cellSize );


    virtual ~PuzzleN();


    // Заполняет поле элементами. Порядок: последовательно.
    void createField();


    // Случайным образом разбрасывает элементы по полю.
    void shuffle();


    // @return Элемент на поле по заданной координате.
    inline element_t const&  element( const logicCoord_t& lc ) const {
        DASSERT( inside( lc ) );
        return mField.at( ic( lc ) );
    }


    // @return 1D-координата пустого элемента.
    inline int emptyElement() const {
        for (auto itr = mField.cbegin(); itr != mField.cend(); ++itr) {
            const element_t element = *itr;
            if (element == EMPTY_ELEMENT) {
                return std::distance( mField.cbegin(), itr );
            }
        }
        DASSERT( false );
        return -1;
    }


    // Рисует в окне Windows.
    void draw( HDC, const RECT& );


    // Отрабатывает перетаскивание мышкой.
    void firstClick( int x, int y );
    void move( int x, int y );
    void stickMove();
    void resetMove();
    void resetFirstClick();
    void resetShift();

    inline void pressMouseButton( bool state ) { mPressMouseButton = state; }
    inline bool pressMouseButton()             { return mPressMouseButton; }


    // Для отладки.
    inline move_t const& aboutMove() const { return mMove; }


private:
    inline bool allFixed() const {
        return (mMove.shift.x == 0) && (mMove.shift.y == 0);
    }


    // @return 1D-индекс элемента, который не зафиксирован или -1, если такого
    //         элемента нет.
    inline int whoUnfixed() const { return mMove.i; }


    // @return Может ли смещаться элемент.
    // @see permitShift()
    inline bool hasPermitShift( int i ) const {
        const auto ps = permitShift( i );
        return (ps.north || ps.south || ps.west || ps.east);
    }


    // @return В каких направлениях может смещаться элемент.
    // # Для пустой ячейки возвращает 'true' по всем направлениям.
    permitShift_t permitShift( int i ) const;


    // @return Наличие соседа у элемента в заданном направении.
    inline bool neighbourNorth( const logicCoord_t& lc ) const {
        const logicCoord_t nlc = { lc.x,  lc.y - 1 };
        return present( nlc );
    }

    inline bool neighbourSouth( const logicCoord_t& lc ) const {
        const logicCoord_t nlc = { lc.x,  lc.y + 1 };
        return present( nlc );
    }

    inline bool neighbourWest( const logicCoord_t& lc ) const {
        const logicCoord_t nlc = { lc.x - 1,  lc.y };
        return present( nlc );
    }

    inline bool neighbourEast( const logicCoord_t& lc ) const {
        const logicCoord_t nlc = { lc.x + 1,  lc.y };
        return present( nlc );
    }


    // @return Наличие элемента по заданным координатам.
    // # Допустимо указывать координаты за пределами поля. В этом случае
    //   ответом будет 'true'.
    inline bool present( const logicCoord_t& lc ) const {
        if ( !inside( lc ) ) { return true; }
        return (element( lc ) != EMPTY_ELEMENT);
    }


    // @return 1D-координата лежит в пределах поля.
    inline bool inside( const logicCoord_t& lc ) const {
        return (lc.x >= 0) && (lc.x < static_cast< int >( N ))
            && (lc.y >= 0) && (lc.y < static_cast< int >( M ));
    }


    // @return 2D-координата, переведёная в 1D.
    inline int ic( const logicCoord_t& c ) const { return ic( c.x, c.y ); }
    inline int ic( int x, int y ) const          { return x + y * N; }

    // @return 1D-координата, переведёная в 2D.
    inline logicCoord_t ci( int i ) const {
        const int y = i / static_cast< int >( N );
        const logicCoord_t c = { i - y * static_cast< int >( N ),  y };
        return c;
    }


    std::unique_ptr< Gdiplus::Bitmap >  picture( const RECT& );
    std::unique_ptr< Gdiplus::Bitmap >  sprite( const element_t& );


private:
    field_t  mField;
    move_t   mMove;

    bool  mPressMouseButton;

    static const element_t  EMPTY_ELEMENT = 0;
};


} // puzzlen
