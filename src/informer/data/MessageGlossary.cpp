#include "MessageGlossary.h"
#include "MessageText.h"
#include "informer/io/TextEscaper.h"
#include "InputMessageSource.h"
#include "informer/io/FileGuard.h"
#include "informer/io/InfosmeException.h"

namespace eyeline {
namespace informer {

MessageGlossary::MessageGlossary( InputMessageSource& ims ) :
log_(smsc::logger::Logger::getInstance("glossary"))
{
    smsc_log_warn(log_,"FIXME: reading glossary at start");
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
                --negMsgId_;
                if (negMsgId_>=0) {
                    negMsgId_ = 0;
                    continue;
                }
                Node* node = hash_.GetPtr(negMsgId_);
                if (node) { continue; } // busy
                hash_.Insert(negMsgId_,
                             Node(list_.insert(list_.begin(),msg),list_.end()));
                msg->id_ = negMsgId_;
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
            Node* node = hash_.GetPtr(msg->id_);
            if (!node) { // new message
                smsc_log_fatal(log_,"msgId=%d not found, should be registered first",msg->id_);
                abort();
                /*
                if (!msg->text_) {
                    smsc_log_fatal(log_,"cannot bind new msg w/o text");
                    abort();
                }
                iter = &hash_.Insert(msg->id_,
                                     list_.insert(list_.begin(),msg));
                msg->ref_ = 2;
                msg->gloss_ = this;
                 */
            } else if (!msg->text_) { // already exist and new has no text, ok
                delete p.ptr_;
                p.ptr_ = *node->iter;
                ++(p.ptr_->ref_);
            } else {
                if ( 0 != strcmp(msg->text_,(*node->iter)->text_) ) {
                    smsc_log_fatal(log_,"bind w/ different text");
                    abort();
                }
                delete p.ptr_;
                p.ptr_ = *node->iter;
                ++(p.ptr_->ref_);
            }
        }
    }
    MessageText* msg = p.get();
    smsc_log_debug(log_,"after bind text='%s' id=%d ref=%u p=%p gloss=%p",
                   msg->text_, msg->id_, msg->ref_, msg, msg->gloss_);
}


void MessageGlossary::registerMessages( InputMessageSource& ims,
                                        TextList&           texts )
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    // adding all messages to the hash and list
    const dlvid_type dlvId = ims.getDlvId();
    for ( TextList::iterator i = texts.begin(); i != texts.end(); ++i ) {
        int32_t msgId = (*i)->id_;
        Node toInsert(i,list_.end());
        if ( msgId < 0 ) {
            registerFailed(texts,i);
            throw InfosmeException("D=%u: negative txtId=%d cannot be registered",dlvId,msgId);
        } else if ( ! (*i)->text_ ) {
            registerFailed(texts,i);
            throw InfosmeException("message w/o text, D=%u, txtId=%d",dlvId,msgId);
        } else if ( msgId == 0 ) {
            // new message, get dynamic id
            if ( ++posMsgId_ <= 0 ) {
                --posMsgId_;
                registerFailed(texts,i);
                throw InfosmeException("D=%u: msgIds exhausted",dlvId);
            }
            msgId = (*i)->id_ = posMsgId_;
        } else {
            // positive msgId
            Node* node = hash_.GetPtr(msgId);
            if (node) {
                if (0 == strcmp((*node->iter)->text_,(*i)->text_)) {
                    registerFailed(texts,i);
                    throw InfosmeException("D=%u: attempt to replace w/ same text",dlvId);
                }
                // different text
                if (++posMsgId_ <= 0 ) {
                    --posMsgId_;
                    registerFailed(texts,i);
                    throw InfosmeException("D=%u: msgIds exhausted on repl",dlvId);
                }
                msgId = (*i)->id_ = posMsgId_;
                toInsert.repl = node->iter;
                node->iter = i;
            } else {
                // explicitly specified textid
                if (msgId>posMsgId_) posMsgId_ = msgId;
            }
        }
        (*i)->ref_ = 1;
        hash_.Insert(msgId,toInsert);
    }
    // inserting texts into glossary file
    smsc::core::buffers::TmpBuf<char,200> buf;
    strcat(makeDeliveryPath(dlvId,buf.get()),"glossary");
    FileGuard fg;
    fg.create((ims.getStorePath()+buf.get()).c_str(),true);
    for ( TextList::iterator i = texts.begin(); i != texts.end(); ++i ) {
        int newpos = sprintf(buf.get(),"%u,%u,\"",(*i)->id_,0);
        if (newpos<=0) {
            registerFailed(texts,texts.end());
            throw InfosmeException("D=%u failed to compose glossary record",dlvId);
        }
        buf.SetPos(size_t(newpos));
        escapeText(buf,(*i)->text_,strlen((*i)->text_));
        buf.Append("\"\n",2);
        fg.write(buf.get(),buf.GetPos());
    }
    fg.close();
    list_.splice(list_.begin(),texts,texts.begin(),texts.end());
}


void MessageGlossary::registerFailed( TextList& texts, TextList::iterator upto )
{
    for ( TextList::iterator j = texts.begin(); j != upto; ++j ) {
        Node n;
        if ( hash_.Pop((*j)->id_,n) && n.repl != list_.end() ) {
            TextList::iterator prev = n.repl;
            Node* next = &n;
            do {
                int32_t txtId = (*next->repl)->id_;
                next = hash_.GetPtr(txtId);
                if (!next) {
                    smsc_log_warn(log_,"repl txtId=%d but node not found",txtId);
                    break;
                }
                next->iter = prev;
            } while (next->repl != list_.end());
        }
    }
    for ( TextList::iterator j = texts.begin(); j != texts.end(); ++j ) {
        delete *j;
    }
    texts.clear();
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
        } else if (ptr->id_>0) {
            smsc_log_warn(log_,"unreffing positive txtId=%d",ptr->id_);
        } else {
            Node node;
            if ( hash_.Pop(ptr->id_,node) ) {
                other = *node.iter;
                list_.erase(node.iter);
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
