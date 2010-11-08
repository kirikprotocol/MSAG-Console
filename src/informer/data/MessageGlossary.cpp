#include <cassert>
#include "MessageGlossary.h"
#include "MessageText.h"
#include "informer/io/TextEscaper.h"
#include "informer/io/FileGuard.h"
#include "informer/io/InfosmeException.h"

namespace eyeline {
namespace informer {

MessageGlossary::MessageGlossary() :
log_(smsc::logger::Logger::getInstance("glossary")),
negTxtId_(0), posTxtId_(0)
{
    smsc_log_debug(log_,"ctor");
}


void MessageGlossary::init( const std::string& storePath,
                            dlvid_type         dlvId )
{
    smsc_log_warn(log_,"reading glossary at init");
    smsc::core::synchronization::MutexGuard mg(lock_);
    smsc::core::buffers::TmpBuf<char,8192> buf;
    strcat(makeDeliveryPath(dlvId,buf.get()),"glossary.txt");
    FileGuard fg;
    try {
        fg.ropen((storePath+buf.get()).c_str());
    } catch ( InfosmeException& e ) {
        smsc_log_info(log_,"D=%u has no glossary",dlvId);
        return;
    }
    // reading glossary
    TextList texts;
    do {
        char* ptr = buf.get();
        const size_t wasread = fg.read( buf.GetCurPtr(), buf.getSize() - buf.GetPos() );
        if (wasread == 0) {
            // EOF
            if (ptr<buf.GetCurPtr()) {
                const size_t trunc = fg.getPos() - (buf.GetCurPtr()-ptr);
                fg.close();
                readGlossaryFailed(storePath,dlvId,trunc,"glossary record is not terminated");
            }
            break;
        }
        buf.SetPos(buf.GetPos()+wasread);
        while ( ptr < buf.GetCurPtr() ) {
            char* end = const_cast<char*>
                (reinterpret_cast<const char*>
                    (memchr(ptr,'\n',buf.GetCurPtr()-ptr)));
            if (!end) break; // EOL not found
            // reading message text
            *end = '\0';
            int shift = 0;
            unsigned txtId, replId;
            sscanf(ptr,"%u,%u,%n",&replId,&txtId,&shift);
            if (!shift) {
                const size_t trunc = fg.getPos() - (buf.GetCurPtr()-ptr);
                fg.close();
                readGlossaryFailed(storePath,dlvId,trunc,"glossary record is broken");
                break;
            }
            // scanning text
            MessageText* mt = new MessageText(unescapeText(ptr+shift,0,end-ptr-shift),txtId);
            mt->ref_ = replId;
            texts.push_back(mt);
            ptr = end+1;
        }
        if ( ptr > buf.get() ) {
            // shifting buffer
            char* o = buf.get();
            const char* i = ptr;
            const char* e = buf.GetCurPtr();
            for ( ; i < e; ) {
                *o++ = *i++;
            }
            buf.SetPos(o-buf.get());
        } else {
            // resize needed
            buf.reserve(buf.getSize()+buf.getSize()/2+100);
        }
    } while (true);
    fg.close();
    try {
        doRegisterMessages(storePath,dlvId,texts);
        // reset refs
        for ( TextList::iterator i = texts.begin(); i != texts.end(); ++i ) {
            (*i)->ref_ = 1;
        }
        list_.splice(list_.begin(),texts,texts.begin(),texts.end());
    } catch (std::exception& e) {
        smsc_log_warn(log_,"FIXME: D=%u glossary read failed, cleanup?: %s", dlvId, e.what());
    }
}


MessageGlossary::~MessageGlossary()
{
    // destroying all messages
    smsc::core::synchronization::MutexGuard mg(lock_);
    hash_.Empty();
    for ( TextList::iterator i = list_.begin(); i != list_.end(); ++i ) {
        if (*i) {
            if ( (*i)->ref_ > 1 || (*i)->id_<=0 ) {
                smsc_log_warn(log_,"wrong ref/id of text='%s' id=%d ref=%u p=%p",
                              (*i)->text_, (*i)->id_, (*i)->ref_, *i );
            }
            delete *i;
        }
    }
    list_.clear();
    posTxtId_ = negTxtId_ = 0;
    smsc_log_debug(log_,"dtor done, list_=%p",&list_);
}


void MessageGlossary::bindText( MessageTextPtr& p )
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
                --negTxtId_;
                if (negTxtId_>=0) {
                    negTxtId_ = 0;
                    continue;
                }
                Node* node = hash_.GetPtr(negTxtId_);
                if (node) { continue; } // busy
                hash_.Insert(negTxtId_,
                             Node(list_.insert(list_.begin(),msg),list_.end()));
                msg->id_ = negTxtId_;
                msg->ref_ = 2;
                msg->gloss_ = this;
                break;
            }
        } else if (msg->id_< 0) {
            // negative id on input
            smsc_log_fatal(log_,"cannot bind negative txtId on input");
            abort();
        } else {
            // msg has positive id
            Node* node = hash_.GetPtr(msg->id_);
            if (!node) { // new message
                smsc_log_fatal(log_,"txtId=%d not found, should be registered first",msg->id_);
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


void MessageGlossary::setTexts( const std::string&  storePath,
                                dlvid_type          dlvId,
                                TextList&           texts )
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    doRegisterMessages(storePath,dlvId,texts);
    if (texts.empty()) return;
    // inserting texts into glossary file
    smsc::core::buffers::TmpBuf<char,200> buf;
    strcat(makeDeliveryPath(dlvId,buf.get()),"glossary.txt");
    FileGuard fg;
    try {
        fg.create((storePath+buf.get()).c_str(),true);
    } catch (std::exception& e) {
        registerFailed(texts,texts.end());
        throw InfosmeException(EXC_SYSTEM,"D=%u failed to create glossary file: %s",dlvId,e.what());
    }
    for ( TextList::iterator i = texts.begin(); i != texts.end(); ++i ) {
        int newpos = sprintf(buf.get(),"%u,%u,",(*i)->id_,(*i)->ref_);
        (*i)->ref_ = 1;
        if (newpos<=0) {
            registerFailed(texts,texts.end());
            throw InfosmeException(EXC_SYSTEM,"D=%u failed to compose glossary record",dlvId);
        }
        buf.SetPos(size_t(newpos));
        try {
            escapeText(buf,(*i)->text_,strlen((*i)->text_));
            *buf.GetCurPtr() = '\n';
            fg.write(buf.get(),buf.GetPos()+1);
        } catch (std::exception& e) {
            registerFailed(texts,texts.end());
            throw InfosmeException(EXC_SYSTEM,"D=%u cannot write glossary record: %s",dlvId,e.what());
        }
    }
    fg.close();
    list_.splice(list_.begin(),texts,texts.begin(),texts.end());
}


void MessageGlossary::getTexts( std::vector< std::string >& texts ) const
{
    {
        MutexGuard mg(lock_);
        texts.reserve(100);
        int id;
        Node* node;
        for ( TextHash::Iterator i(hash_); i.Next(id,node); ) {
            Node* orig = node;
            int32_t origId = id;
            while (orig->repl != list_.end()) {
                // this is a replacement node, try to find the original node
                origId = (*orig->repl)->id_;
                orig = hash_.GetPtr(origId);
                if (!orig) {
                    throw InfosmeException(EXC_LOGICERROR,"glossary corrupted at id=%u repl=%u",id,origId);
                }
            }
            smsc_log_debug(log_,"got node=%d orig=%d",id,origId);
            if (origId<=0) {
                throw InfosmeException(EXC_LOGICERROR,"invalid origId=%d",origId);
            }
            if (unsigned(origId) > texts.size()) {
                texts.resize(origId);
            }
            if ( texts[origId-1].empty() ) {
                texts[origId-1] = (*node->iter)->text_;
            }
        }
    }
    for ( std::vector< std::string >::iterator i = texts.begin();
          i != texts.end(); ++i ) {
        if ( i->empty() ) {
            throw InfosmeException(EXC_LOGICERROR,"empty glossary element at %u",
                                   unsigned(std::distance(texts.begin(),i)+1));
        }
    }
}


void MessageGlossary::doRegisterMessages( const std::string&    storePath,
                                          dlvid_type            dlvId,
                                          TextList&             texts )
{
    // adding all messages to the hash and list
    for ( TextList::iterator i = texts.begin(); i != texts.end(); ) {
        int32_t txtId = (*i)->id_;
        int32_t replId = (*i)->ref_;
        Node toInsert(i,list_.end());
        if ( txtId < 0 ) {
            registerFailed(texts,i);
            throw InfosmeException(EXC_LOGICERROR,"D=%u: negative txtId=%d cannot be registered",dlvId,txtId);
        } else if ( ! (*i)->text_ ) {
            registerFailed(texts,i);
            throw InfosmeException(EXC_LOGICERROR,"message w/o text, D=%u, txtId=%d",dlvId,txtId);
        } else if ( txtId == 0 ) {
            // new message, get dynamic id
            if ( ++posTxtId_ <= 0 ) {
                --posTxtId_;
                registerFailed(texts,i);
                throw InfosmeException(EXC_SYSTEM,"D=%u: txtIds exhausted",dlvId);
            }
            txtId = (*i)->id_ = posTxtId_;
            replId = 0;
        } else {
            // positive txtId
            Node* node = hash_.GetPtr(txtId);
            if (node) {
                if (0 == strcmp((*node->iter)->text_,(*i)->text_)) {
                    smsc_log_debug(log_,"D=%u: not adding text id=%u w/ the same text='%s'",
                                   dlvId,txtId,(*i)->text_);
                    i = texts.erase(i);
                    continue;
                }
                // different text, try to get txtId from message itself
                if (replId) {
                    // replacement id
                    if ( hash_.GetPtr(replId) ) {
                        registerFailed(texts,i);
                        throw InfosmeException(EXC_LOGICERROR,"D=%u: replacement id=%d already registered",dlvId,replId);
                    }
                    std::swap(txtId,replId);
                    if (posTxtId_<txtId) { posTxtId_ = txtId; }
                } else {
                    if (++posTxtId_ <= 0 ) {
                        --posTxtId_;
                        registerFailed(texts,i);
                        throw InfosmeException(EXC_SYSTEM,"D=%u: txtIds exhausted on repl",dlvId);
                    }
                    replId = txtId;
                    txtId = posTxtId_;
                }
                (*i)->id_ = txtId;
                toInsert.repl = node->iter;
                node->iter = i;
            } else {
                // node not found, but txtId is specified
                if (txtId>posTxtId_) posTxtId_ = txtId;
                replId = 0;
            }
        }
        (*i)->ref_ = replId;
        (*i)->gloss_ = this;
        hash_.Insert(txtId,toInsert);
        ++i;
    }
}


void MessageGlossary::registerFailed( TextList& texts, TextList::iterator upto )
{
    for ( TextList::reverse_iterator j(upto); j != texts.rend(); ++j ) {
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
        (*j)->ref_ =0; // to weed dtor warning
        delete *j;
    }
    texts.clear();
}


void MessageGlossary::readGlossaryFailed( const std::string& storePath,
                                          dlvid_type         dlvId,
                                          size_t             trunc,
                                          const char*        msg )
{
    smsc_log_warn(log_,"D=%u %s at %llu, to be truncated",dlvId,msg,ulonglong(trunc));
    char buf[100];
    strcat(makeDeliveryPath(dlvId,buf),"glossary.txt");
    FileGuard fg;
    fg.create((storePath+buf).c_str());
    fg.truncate(trunc);
}


void MessageGlossary::ref( MessageText* ptr )
{
    {
        MutexGuard mg(lock_);
        ++(ptr->ref_);
    }
    smsc_log_debug(log_,"ref text='%s' id=%d ref=%u p=%p",
                   ptr->text_, ptr->id_, ptr->ref_, ptr);
    assert(ptr->gloss_ == this);
}


void MessageGlossary::unref( MessageText* ptr )
{
    smsc_log_debug(log_,"unref text='%s' id=%d ref=%u p=%p gloss=%p",
                   ptr->text_, ptr->id_, ptr->ref_, ptr, ptr->gloss_ );
    MessageText* other = 0;
    do {
        MutexGuard mg(lock_);
        --ptr->ref_;
        // ref must be 1,2,3,etc
        assert(ptr->ref_>0);
        if (ptr->ref_>1 || ptr->id_>0) {
            // still reffed or positive
            ptr = 0;
            break;
        }
        Node node;
        if ( hash_.Pop(ptr->id_,node) ) {
            other = *node.iter;
            assert(node.repl == list_.end());
            list_.erase(node.iter);
        } else {
            other = ptr;
            ptr = 0;
        }
    } while (false);

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
