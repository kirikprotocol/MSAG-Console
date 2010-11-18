/*

#include <cstdlib>
#include "MessageText.h"
#include "MessageGlossary.h"

namespace {
smsc::logger::Logger* log_ = 0;
smsc::core::synchronization::Mutex lock_;

void getlog()
{
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("msgtext");
    }
}

}

namespace eyeline {
namespace informer {

MessageText::MessageText( const char* text, int32_t id ) :
text_(text?copyText(text):0), gloss_(0), id_(id), ref_(0)
{
    ::getlog();
    smsc_log_debug(log_,"+text %p, text=%p",this,text_);
}


MessageText::~MessageText()
{
    smsc_log_debug(log_,"-text %p, text=%p id=%d ref=%u",this,text_,id_,ref_);
    if (text_) delete[] text_;
}


MessageTextPtr::MessageTextPtr( MessageText* ptr ) : ptr_(ref(ptr))
{
    ::getlog();
    smsc_log_debug(log_,"+textptr p=%p",this);
}


MessageTextPtr::MessageTextPtr( const MessageTextPtr& ptr ) : ptr_(copy(ptr.ptr_))
{
    smsc_log_debug(log_,"+textptr p=%p",this);
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
    smsc_log_debug(log_,"-textptr p=%p",this);
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
            ptr->gloss_->ref(ptr);
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
            ptr->gloss_->ref(ptr);
        }
    }
    return ptr;
}


void MessageTextPtr::unref( MessageText* ptr )
{
    if (ptr) {
        if (ptr->gloss_) {
            ptr->gloss_->unref(ptr);
        } else {
            // no glossary - no locking
            if (ptr->ref_>0) {
                smsc_log_fatal(log_,"textptr=%p w/o gloss text='%s' id=%d ref=%u p=%p, wrong ref!",
                               this, ptr->text_, ptr->id_, ptr->ref_, ptr );
                abort();
            }
            delete ptr;
        }
    }
}

}
}

 */
