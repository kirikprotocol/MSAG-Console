//
// A test of allocation on dedicated buffer
//

#include <cstdio>
#include <string>

struct A
{
    explicit A( const std::string& a ) : a_(a) { printf("created %p %s\n", this, a_.c_str()); }
    virtual ~A() { printf("destroyed %p %s\n", this, a_.c_str() ); }
private:
    A();
    A( const A& a );
    A& operator = ( const A& a );
private:
    std::string a_;
};


int main()
{
    std::auto_ptr< A > a( new A("simple new") );

    char buf[sizeof(A)];
    A* b( new (buf) A("on buf") );
    printf("some actions...\n");
    b->~A();
    return 0;
}
