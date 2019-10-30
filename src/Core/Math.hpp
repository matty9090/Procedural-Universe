#pragma once

namespace Maths
{
    template <class T>
    T Lerp(T a, T b, T t)
    {
        return a + t * (b - a);
    }
}