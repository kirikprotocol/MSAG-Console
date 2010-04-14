#include <iostream>

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
    return 0;
}
