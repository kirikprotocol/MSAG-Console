#include <iostream>
#include <string>
#include <memory>
#include "core/buffers/XTree.hpp"

using namespace smsc::core::buffers;

class A
{
public:
    A( const std::string& val ) : val_(val) {
        printf("+A@%p(%s)\n",this,val.c_str());
    }
    A( const A& a ) : val_(a.val_) {
        printf("+A@%p(A@%p(%s))\n",this,&a,val_.c_str());
    }
    A& operator = ( const A& a ) {
        if ( &a != this ) {
            val_ = a.val_;
            printf("A@%p = A@%p(%s)\n",this,&a,val_.c_str());
        }
        return *this;
    }
    ~A() {
        printf("~A@%p(%s)\n",this,val_.c_str());
    }
private:
    std::string val_;
};


struct Keep {
public:
    typedef XTree< A, HeapAllocator, false > XTreeType;
    
    Keep() : xtree_( new XTreeType ) {}

    ~Keep() {
        printf("destroying xtree\n");
        xtree_.reset(0);
        printf("xtree destroyed\n");
        // std::for_each( keep_.begin(), keep_.end(), PtrDestroy() );
    }
    
    void add( const char* name ) {
        A a(name);
        // keep_.push_back(a);
        printf("adding A@%p(%s) to xtree\n",&a,name);
        xtree_->Insert(name,a);
    }

private:
    // std::vector< A* >        keep_;
    std::auto_ptr<XTreeType> xtree_;
};



int main()
{
    Keep keep;
    keep.add(".1.1.7913???????");
    keep.add(".1.1.7913765????");
    keep.add(".0.1.89137654079");
    return 0;
}
