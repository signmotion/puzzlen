#include "../include/stdafx.h"
#include "../include/PuzzleN.h"
#include <assert.h>


namespace puzzlen {


PuzzleN::PuzzleN( size_t n, size_t m, size_t cellSize ) :
    N( n ), M( m ),
    cellSize( cellSize ),
    mPressMouseButton( false ),
    glueDistance( cellSize * GLUE_PERCENT / 100 )
{
    ASSERT( ((n > 1) && (m > 1))
        && "Width and height of puzzle must have value above 1." );
    ASSERT( ( (cellSize >= 10) && (cellSize <= 100) )
        && "Size of cell must have value between [10; 100]." );

    createField();

    resetMove();
}




PuzzleN::~PuzzleN() {
}




void
PuzzleN::createField() {

    mField.reserve( N * M );
    for (int y = 0; y < static_cast< int >( M ); ++y) {
        for (int x = 0; x < static_cast< int >( N ); ++x) {

            // # ѕоследн€€ €чейка должна остатьс€ пустой
            const int i = ic( x, y );
            if (i == (M * N - 1)) {
                mField.push_back( EMPTY_ELEMENT );
                break;
            }

            const element_t element = static_cast< size_t >( i + 1 );
            mField.push_back( element );

        } // for (size_t x = 0; ...
    } // for (size_t y = 0; ...
}




void
PuzzleN::shuffle() {
    std::srand( static_cast< unsigned int >( time( nullptr ) ) );
    std::random_shuffle( mField.begin(), mField.end() );
    resetMove();
}




void
PuzzleN::draw( HDC hdc,  const RECT& rc ) {

    using namespace Gdiplus;

    auto p = picture( rc );
    Graphics  g( hdc, false );
    const Rect  dest( 0, 0, p->GetWidth(), p->GetHeight() );
    g.DrawImage( p.get(), dest, 0, 0, p->GetWidth(), p->GetHeight(), UnitPixel );
}




std::unique_ptr< Gdiplus::Bitmap >
PuzzleN::sprite( const element_t&  element ) {

    using namespace Gdiplus;

    DASSERT( element != EMPTY_ELEMENT );

    std::unique_ptr< Bitmap >  s( new Bitmap(
        cellSize, cellSize, PixelFormat32bppPARGB
    ) );

    Graphics  g( s.get() );
    //g.Clear( 0xffffffff );
#if 0
    g.SetCompositingMode( CompositingModeSourceOver );
    g.SetCompositingQuality( CompositingQualityHighSpeed );
    g.SetPixelOffsetMode( PixelOffsetModeHighSpeed );
    g.SetSmoothingMode( SmoothingModeHighSpeed );
    g.SetInterpolationMode( InterpolationModeHighQuality );
#else
    // быстрее
    g.SetCompositingMode( CompositingModeSourceOver );
    g.SetCompositingQuality( CompositingQualityHighSpeed );
    g.SetPixelOffsetMode( PixelOffsetModeHalf );
    g.SetSmoothingMode( SmoothingModeHighSpeed );
    g.SetInterpolationMode( InterpolationModeNearestNeighbor );
#endif
    g.SetTextRenderingHint( TextRenderingHintAntiAliasGridFit );
    g.SetPageUnit( UnitPixel );

    const RectF
        bounds( 0, 0, float( s->GetWidth() ), float( s->GetHeight() ) );

    // загружаем фоновое изображение, раст€гиваем на всю €чейку
    const std::wstring file = PATH_MEDIA + L"/cell.png";
    Image  image( file.c_str() );
    //ASSERT( (bg.GetType() != ImageTypeUnknown)
    //    && "Image for cell not found." );
    g.DrawImage( &image, bounds );

#if 0
    // пишем градиентом
    const Color  a( rand() % 255,  rand() % 255,  rand() % 255 );
    const Color  b( 255 - a.GetG(),  255 - a.GetB(),  255 - a.GetR() );
    const LinearGradientBrush
        brush( bounds, a, b, LinearGradientModeBackwardDiagonal );
#else
    const Color  a( 0xA9, 0, 0 );
    const SolidBrush brush( a );
#endif
    
    StringFormat  format;
    format.SetAlignment( StringAlignmentCenter );
    format.SetLineAlignment( StringAlignmentCenter );
    const Font  font( L"Arial", 14, FontStyleBold );

    std::wostringstream  ss;
    ss << element;
    g.DrawString( ss.str().c_str(), -1, &font, bounds, &format, &brush );

    return std::move( s );
}




std::unique_ptr< Gdiplus::Bitmap >
PuzzleN::picture( const RECT& rc ) {

    using namespace Gdiplus;

    std::unique_ptr< Bitmap >  p( new Bitmap(
        rc.right  - rc.left,
        rc.bottom - rc.top,
        PixelFormat32bppPARGB
    ) );
    Graphics  g( p.get() );
    g.Clear( 0xffffffff );

    for (auto itr = mField.cbegin(); itr != mField.cend(); ++itr) {

        const element_t element = *itr;
        if (element == EMPTY_ELEMENT) {
            continue;
        }

        const size_t i = std::distance( mField.cbegin(), itr );
        const logicCoord_t lc = ci( i );

        // один из элементов мог быть смещЄн
        const bool shifted = (i == mMove.i);

        const auto s = sprite( element );
        const int cx = lc.x * cellSize + (shifted ? mMove.shift.x : 0);
        const int cy = lc.y * cellSize + (shifted ? mMove.shift.y : 0);
        const Rect  dest( cx, cy, s->GetWidth(), s->GetHeight() );
        g.DrawImage( s.get(), dest, 0, 0, s->GetWidth(), s->GetHeight(), UnitPixel );

    } // for (auto itr = mField.cbegin(); ...

    return std::move( p );
}




void
PuzzleN::firstClick( int x, int y ) {

    // определим элемент, который собираютс€ т€нуть
    const logicCoord_t lc = { x / cellSize,  y / cellSize };
    const int i = ic( lc );
    DASSERT( (i >= 0) || (i < static_cast< int >( mField.size() )) );

    // все элементы на фиксированных позици€х?
    const auto ei = emptyElement();
    if ( !allFixed() ) {
        // не зафиксирован именно этот элемент? пытаютс€ двинуть пустой?
        if ( (i != whoUnfixed()) && (i != ei) ) {
            return;
        }
    }

    // элемент может перемещатьс€?
    if ( !hasPermitShift( i ) ) {
        return;
    }
    
    // запомним, какой элемент *начинает* перемещение
    if ( (mMove.i == -1) || (i == whoUnfixed()) || (i == ei) ) {
        // сохраним старое смещение дл€ элемента
        mMove.firstClick.x = x - mMove.shift.x;
        mMove.firstClick.y = y - mMove.shift.y;
        if (i == ei) {
            // захват был со стороны пустого элемента
            const logicCoord_t elc = ci( mMove.i );
            const logicCoord_t delta = { lc.x - elc.x,  lc.y - elc.y };
            mMove.emptyClickShift.x = delta.x * cellSize;
            mMove.emptyClickShift.y = delta.y * cellSize;
            mMove.firstClick.x += mMove.emptyClickShift.x;
            mMove.firstClick.y += mMove.emptyClickShift.y;

        } else {
            mMove.emptyClickShift.x = mMove.emptyClickShift.y = 0;
            mMove.i = i;
        }
    }
}




void
PuzzleN::move( int x, int y ) {

    if (mMove.i == -1) {
        // элемент дл€ перемещени€ не выбран
        return;
    }

    if (mMove.firstClick.x == -1) {
        // щелчок был не на этом элементе
        return;
    }

    if ( !mPressMouseButton ) {
        // клавиша мыши не опущена
        return;
    }

    const auto ps = permitShift( mMove.i );
    const int  sx = x - mMove.firstClick.x + mMove.emptyClickShift.x;
    const int  sy = y - mMove.firstClick.y + mMove.emptyClickShift.y;
    mMove.shift.x = ((ps.west  && (sx < 0)) || (ps.east  && (sx > 0))) ? sx : 0;
    mMove.shift.y = ((ps.north && (sy < 0)) || (ps.south && (sy > 0))) ? sy : 0;
    if (std::abs( mMove.shift.x ) > static_cast< int >( cellSize )) {
        mMove.shift.x = cellSize * ((mMove.shift.x < 0) ? -1 : 1);
    }
    if (std::abs( mMove.shift.y ) > static_cast< int >( cellSize )) {
        mMove.shift.y = cellSize * ((mMove.shift.y < 0) ? -1 : 1);
    }
}




void
PuzzleN::stickMove() {

    if (mMove.i == -1) {
        // элемент дл€ перемещени€ не выбран
        return;
    }

    // зафиксируем перемещение элемента
    // реализуем "прилипание" (дл€ удобства)
    const size_t dx = std::abs( mMove.shift.x );
    const size_t dy = std::abs( mMove.shift.y );
    const auto ps = permitShift( mMove.i );
    const bool horizontal = (ps.west  || ps.east);
    bool glueX = false;
    bool glueY = false;
    if ( horizontal ) {
        glueX = (dx <= glueDistance) || ((cellSize - dx) <= glueDistance);
    } else {
        glueY = (dy <= glueDistance) || ((cellSize - dy) <= glueDistance);
    }

    // # Ёлемент умеет оставатьс€ в промежуточной позиции.
    if ( !glueX && !glueY ) {
        return;
    }

    // # Ёлемент может помен€тьс€ местами только с пустым элементом.
    const bool change =
        (glueX && ((cellSize - dx) <= glueDistance))
     || (glueY && ((cellSize - dy) <= glueDistance));
    if ( change ) {
        // обмен€ем с пустым
        const int emptyI = emptyElement();
        std::swap( mField.at( mMove.i ),  mField.at( emptyI ) );
    }

    mMove.i = -1;
    resetShift();
}




void
PuzzleN::resetMove() {
    static const move_t EMPTY_MOVE = {
        // i
        -1,
        // firstClick
        { -1, -1 },
        // shift
        { 0, 0 },
        // emptyClickShift
        { 0, 0 }
    };
    mMove = EMPTY_MOVE;
}




void
PuzzleN::resetFirstClick() {
    mMove.firstClick.x = mMove.firstClick.y = -1;
}




void
PuzzleN::resetShift() {
    mMove.shift.x = mMove.shift.y = 0;
}




PuzzleN::permitShift_t
PuzzleN::permitShift( int i ) const {

    if (i == emptyElement()) {
        static const permitShift_t PS = { true, true, true, true };
        return PS;
    }

    const logicCoord_t lc = ci( i );
    const permitShift_t ps = {
        !neighbourNorth( lc ),  !neighbourSouth( lc ),
        !neighbourWest( lc ),   !neighbourEast( lc )
    };
    // # Ёлемент может смещатьс€ только по вертикали или горизонтали.
    ASSERT( ( !(ps.north || ps.south || ps.west || ps.east)
            || ( (ps.north || ps.south) && !(ps.west || ps.east))
            || (!(ps.north || ps.south) &&  (ps.west || ps.east))
        ) && "Permit vertical or horisontal shift only." );

    return ps;
}


} // puzzlen
