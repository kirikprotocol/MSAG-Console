#include <iostream>

// template concepts

struct Base32
{
    int32_t data_;
};
struct Base64
{
    int64_t data_;
};

template < unsigned SPEC > struct BaseSelector
{
    typedef Base64 Base;
};

template <> struct BaseSelector<1000U>
{
    typedef Base32 Base;
};


template < unsigned SPEC > struct Derived : public BaseSelector< SPEC >::Base
{
    Derived() { this->data_ = 0; }
};


template <class T> struct A
{
    A() {
        std::cout << "A::A()" << std::endl;
    }
    void process() {
        std::cout << "A<T>::process()" << std::endl;
    }
};

template <> void A<int>::process()
{
    std::cout << "A<int>::process()" << std::endl;
}


int main()
{
    std::cout << "Possibility to specialize one member of a class template only" << std::endl;
    A<float> af;
    af.process();
    A<int> ai;
    ai.process();

    std::cout << "selecting base class" << std::endl;
    std::cout << "sizeof(Derived<1000U>)=" << sizeof(Derived<1000U>) << std::endl;
    std::cout << "sizeof(Derived<2000U>)=" << sizeof(Derived<2000U>) << std::endl;
    return 0;
}
