namespace ns1 {
    namespace sub {
        struct A {
            int data;
        };
    }
}

namespace ns2 {
    namespace sub = ns1::sub;

    struct B {
        sub::A a;
    };
}


int main()
{
    ns2::B b;
    ns2::sub::A a;
    a.data = 100;
    b.a.data = a.data;
    return 0;
}
