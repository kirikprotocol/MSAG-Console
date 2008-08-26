#include <memory>
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


std::auto_ptr< A > fun( const std::string& where )
{
    return std::auto_ptr<A>(new A(where));
}

void fun2( bool take, 
           std::auto_ptr< A >& arg )
{
    if ( take ) {
        printf("taking %p\n", arg.get() );
        arg.reset(0);
    }
    printf("fun2 finishing\n");
}

int main()
{
    std::auto_ptr< A > a;
    a = fun("false");
    fun2( false, a );
    a = fun("true");
    fun2( true, a );
    return 0;
}
