#include <iostream>
#include <memory>

namespace outer {

namespace inner {

struct A {
    virtual void exec() {
        std::cout << "A::exec()" << std::endl;
    }
};

}

struct B : public inner::A 
{
    void exec() {
        std::cout << "B::exec()" << std::endl;
    }
};

struct C {
    void exec()
    {
        // A test on how to refer to base class method.
        // It was found that both cases are ok.

        // 1.
        b.inner::A::exec();

        // 2. is not working for CC
        // b.A::exec();
    }
    B b;
};

}

int main()
{
    outer::C c;
    c.exec();
    return 0;
}
