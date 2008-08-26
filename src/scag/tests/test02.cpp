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


void fun( bool skip )
{
    char buf[sizeof(A)];
    std::auto_ptr<A> a;
    if ( ! skip ) a.reset( new (&buf) A("locked") );

    B b("in fun");
}


int main()
{
    fun( false );
    fun( true );
    return 0;
}
