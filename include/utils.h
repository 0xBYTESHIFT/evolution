#pragma once
#include <cmath>

namespace math{
    template<class Arg>
    inline static Arg pif(const Arg &x0, const Arg &y0, const Arg &x1, const Arg &y1){
        return std::sqrt(std::pow(x0-x1, 2) + std::pow(y0-y1, 2));
    };
    inline static const float pi = std::acos(-1.f);
};