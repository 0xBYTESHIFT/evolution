#pragma once

template<class T>
class property{
protected:
    T m_value;
public:
    using value_type = T;

    property(){}

    template<class Arg>
    property(Arg &&rhs):m_value(std::forward<Arg>(rhs)) {}

    auto& operator ()()     { return m_value; }
    auto& operator ()()const{ return m_value; }
    auto& operator ->()     { return m_value(); }
    auto& operator ->()const{ return m_value(); }
    auto& set(const T& rhs) { return m_value = rhs; }
    auto& set(T&& rhs)      { return m_value = std::move(rhs); }
    auto& get()             { return m_value; }
    auto& get()const        { return m_value; }

    operator T()const      { return m_value; }
    auto operator =(const T& rhs){ return m_value = rhs; }
    auto operator =(T&& rhs){ return m_value = std::move(rhs); }
    //template<class Arg> auto& operator =  (Arg&& rhs) { return m_value  = std::forward<Arg>(rhs); }
};