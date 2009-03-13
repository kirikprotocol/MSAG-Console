#include <iostream>
#include <memory>

struct A
{
public:
    virtual ~A() {
        shutdown();
        std::cout << "A::~A()" << std::endl;
    }
    virtual void shutdown() = 0;
};

struct B : public A
{
public:
    virtual ~B() {
        std::cout << "B::~B()" << std::endl;
    }
    virtual void shutdown() {
        std::cout << "destroying B" << std::endl;
    }
};


int main()
{
    std::auto_ptr<A> b(new B);
    std::cout << "object @" << b.get() << std::endl;
    return 0;
}
