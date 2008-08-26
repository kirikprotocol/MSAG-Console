#include <string>
#include <cstdlib>

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

struct B : public A
{
    B( const std::string& a ) : A(a) {}
    virtual ~B() {}
};


int main()
{
    A a("a");
    B b("b");
    std::auto_ptr< B >( new B("btemp") );
    std::auto_ptr< A > c( new B("c=anew") );
    c.get();
    return 0;
}
