#include <cstdlib>
#include <cassert>
#include "MessageText.h"

namespace eyeline {
namespace informer {

MessageText::MessageText( const char* text, int32_t id ) :
text_(copyText(text)), gloss_(0), id_(id), ref_(0)
{
}


MessageTextPtr::MessageTextPtr( MessageText* ptr ) : ptr_(ref(ptr))
{
}

MessageTextPtr::MessageTextPtr( MessageTextPtr& ptr ) : ptr_(copy(ptr.ptr_))
{
}

MessageTextPtr& MessageTextPtr::operator = ( const MessageTextPtr& ptr )
{
    if (ptr_ != ptr.ptr_) {
        unref(ptr_);
        ptr_ = copy(ptr.ptr_);
    }
    return *this;
}


MessageTextPtr::~MessageTextPtr()
{
    unref(ptr_);
}


void MessageTextPtr::reset( MessageText* ptr )
{
    if (ptr!=ptr_) {
        unref(ptr_);
        ptr_ = ref(ptr);
    }
}


MessageText* MessageTextPtr::copy( MessageText* ptr )
{
    if (ptr) {
        if (ptr->gloss_) {
            // glossary, need lock&ref
            abort(); // not impl
        } else {
            // simply copying
            ptr = new MessageText(ptr->getText(),ptr->getTextId());
        }
    }
    return ptr;
}


MessageText* MessageTextPtr::ref( MessageText* ptr )
{
    if (ptr) {
        if (ptr->gloss_) {
            // we have glossary, need to ref
            abort(); // not impl
        }
    }
    return ptr;
}


void MessageTextPtr::unref( MessageText* ptr )
{
    if (ptr) {
        if (ptr->gloss_) {
            // need lock&unref
            abort();
        } else {
            // no glossary - no locking
            assert(ptr->ref_<=0);
            delete ptr;
        }
    }
}

}
}
