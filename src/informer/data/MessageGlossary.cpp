#include "MessageGlossary.h"
#include "MessageText.h"

namespace eyeline {
namespace informer {

MessageGlossary::MessageGlossary() :
log_(smsc::logger::Logger::getInstance("glossary"))
{
}


void MessageGlossary::bindMessage( MessageTextPtr& p )
{
    if (!p.get()) {
        smsc_log_fatal(log_,"ptr is not valid");
        abort();
    }
    {
        MessageText* msg = p.get();
        if (msg->gloss_) {
            smsc_log_fatal(log_,"glossary already bound");
            abort();
        }
        smsc_log_debug(log_,"bind text='%s' id=%d ref=%u p=%p",
                       msg->text_, msg->id_, msg->ref_, msg);
        MutexGuard mg(lock_);
        if ( !msg->id_ ) {
            if (!msg->text_) {
                smsc_log_fatal(log_,"cannot insert dynamic msg w/o text");
                abort();
            }
            // message is new to glossary, assign dynamic id
            while (true) {
                --lastMsgId_;
                if (lastMsgId_>=0) {
                    lastMsgId_ = 0;
                    continue;
                }
                TextList::iterator* iter = hash_.GetPtr(lastMsgId_);
                if (iter) { continue; } // busy
                hash_.Insert(lastMsgId_,
                             list_.insert(list_.begin(),msg));
                msg->id_ = lastMsgId_;
                msg->ref_ = 1;
                msg->gloss_ = this;
                break;
            }
        } else if (msg->id_< 0) {
            // negative id on input
            smsc_log_fatal(log_,"cannot bind negative msgid on input");
            abort();
        } else {
            // msg has positive id
            TextList::iterator* iter = hash_.GetPtr(msg->id_);
            if (!iter) { // new message
                if (!msg->text_) {
                    smsc_log_fatal(log_,"cannot bind new msg w/o text");
                    abort();
                }
                hash_.Insert(msg->id_,
                             list_.insert(list_.begin(),msg));
                msg->ref_ = 2;
                msg->gloss_ = this;
            } else if (!msg->text_) { // already exist and new has no text, ok
                delete p.ptr_;
                p.ptr_ = **iter;
                ++(p.ptr_->ref_);
            } else {
                if ( 0 != strcmp(msg->text_,(**iter)->text_) ) {
                    smsc_log_fatal(log_,"bind w/ different text");
                    abort();
                }
                delete p.ptr_;
                p.ptr_ = **iter;
                ++(p.ptr_->ref_);
            }
        }
    }
    MessageText* msg = p.get();
    smsc_log_debug(log_,"after bind text='%s' id=%d ref=%u p=%p gloss=%p",
                   msg->text_, msg->id_, msg->ref_, msg, msg->gloss_);
}


void MessageGlossary::ref( MessageText* ptr )
{
    smsc_log_debug(log_,"ref text='%s' id=%d ref=%u p=%p gloss=%p",
                   ptr->text_, ptr->id_, ptr->ref_, ptr, ptr->gloss_ );
    MutexGuard mg(lock_);
    ++(ptr->ref_);
}


void MessageGlossary::unref( MessageText* ptr )
{
    smsc_log_debug(log_,"unref text='%s' id=%d ref=%u p=%p gloss=%p",
                   ptr->text_, ptr->id_, ptr->ref_, ptr, ptr->gloss_ );
    MessageText* other = 0;
    {
        MutexGuard mg(lock_);
        if (ptr->ref_>1) {
            --(ptr->ref_);
            ptr = 0;
        } else {
            TextList::iterator iter;
            if ( hash_.Pop(ptr->id_,iter) ) {
                other = *iter;
                list_.erase(iter);
            } else {
                other = ptr;
                ptr = 0;
            }
        }
    }
    if (other) {
        if (!ptr) {
            smsc_log_error(log_,"id=%d ptr=%p is not in the texts",
                           ptr->id_, ptr );
        } else if (other != ptr) {
            smsc_log_error(log_,"id=%d other=%p is not equal to ptr=%p, destroy",
                           ptr->id_, other, ptr);
            delete other;
        }
    }
    if (ptr) {
        smsc_log_debug(log_,"destroy p=%p",ptr);
        delete ptr;
    }
}

}
}
