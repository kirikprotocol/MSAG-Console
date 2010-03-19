#include <iostream>

template <class T> struct A
{
    A() {
        std::cout << "ctor" << std::endl;
    }
    void process() {
        std::cout << "A" << std::endl;
    }
};

template <> void A<int>::process()
{
    std::cout << "A<int>" << std::endl;
}


int main()
{
    A<float> af;
    af.process();
    A<int> ai;
    ai.process();
    return 0;
}
