#pragma once

#include "configure.h"


namespace puzzlen {


class Exception :
    public std::exception
{
public:
    inline Exception( const std::string& s ) :
        std::exception( s.c_str() )
    {
        ASSERT( !s.empty() );
    }
};


} // puzzlen
