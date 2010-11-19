#include <cassert>
#include "MessageGlossary.h"
#include "MessageText.h"
#include "CommonSettings.h"
#include "informer/io/TextEscaper.h"
#include "informer/io/FileGuard.h"
#include "informer/io/InfosmeException.h"

namespace eyeline {
namespace informer {


class MessageGlossary::ChangeGuard
{
public:
    ChangeGuard( const MessageGlossary& g ) : mg_(g) {
        while (true) {
            MutexGuard mg(mg_.lock_);
            if (!mg_.changing_) { break; }
            mg_.lock_.wait(100);
        }
    }

    ~ChangeGuard() {
        MutexGuard mg(mg_.lock_);
        mg_.changing_ = false;
        mg_.lock_.notify();
    }
private:
    ChangeGuard(const ChangeGuard& );
    ChangeGuard& operator = (const ChangeGuard& );
    const MessageGlossary& mg_;
};


MessageGlossary::MessageGlossary() :
log_(smsc::logger::Logger::getInstance("glossary")),
dlvId_(0),
hash_(0),
maxRealId_(0),
changing_(false)
{
    smsc_log_debug(log_,"ctor");
}


MessageGlossary::~MessageGlossary()
{
    // destroying all messages
    delete hash_; hash_ = 0;
    for ( TextList::iterator i = list_.begin(); i != list_.end(); ++i ) {
        delete i->getText();
    }
    list_.clear();
    smsc_log_debug(log_,"D=%u dtor done",dlvId_);
}


void MessageGlossary::init( dlvid_type dlvId )
{
    dlvId_ = dlvId;
    smsc_log_warn(log_,"D=%u reading glossary at init",dlvId_);
    smsc::core::synchronization::MutexGuard mg(lock_);
    smsc::core::buffers::TmpBuf<char,8192> buf;
    strcat(makeDeliveryPath(buf.get(),dlvId_),"glossary.txt");
    FileGuard fg;
    try {
        fg.ropen((getCS()->getStorePath()+buf.get()).c_str());
    } catch ( ErrnoException& e ) {
        smsc_log_info(log_,"D=%u has no glossary",dlvId);
        return;
    }
    // reading glossary
    unsigned realId = 1;
    int maxInputId = 0;
    std::auto_ptr<TextHash> hash(new TextHash);
    do {
        char* ptr = buf.get();
        const size_t wasread = fg.read( buf.GetCurPtr(), buf.getSize() - buf.GetPos() );
        if (wasread == 0) {
            // EOF
            if (ptr<buf.GetCurPtr()) {
                // const size_t trunc = fg.getPos() - (buf.GetCurPtr()-ptr);
                readGlossaryFailed(realId,"glossary record is not terminated");
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
            int inputId;
            sscanf(ptr,"%d,%n",&inputId,&shift);
            if (!shift) {
                readGlossaryFailed(realId,"glossary record is broken");
            }
            if (inputId<=0 || inputId>=maxInputId+1) {
                readGlossaryFailed(realId,"glossary record has invalid replacement id");
            }
            // scanning text
            const char* text = unescapeText(ptr+shift,0,end-ptr-shift);
            TextList::iterator i = list_.insert(list_.end(),Node(text,realId));
            hash->Insert(realId,i);
            hash->Insert(-inputId,i);
            if (inputId>maxInputId) { maxInputId = inputId; }
            maxRealId_ = realId;
            ++realId;
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
    hash_ = hash.release();
}


void MessageGlossary::fetchText( MessageText& p, bool input, bool returnRealId )
{
    int32_t id = p.getTextId();
    if (id<=0) {
        throw InfosmeException(EXC_LOGICERROR,"D=%u cannot fetch text for negative id=%d",dlvId_,id);
    } else if (p.getText()) {
        throw InfosmeException(EXC_LOGICERROR,"D=%u text is already bound",dlvId_);
    }
    {
        smsc_log_debug(log_,"D=%u fetch %s text id=%u",
                       dlvId_, input ? "input": "real", id);
        MutexGuard mg(lock_);
        if (!hash_) {
            throw InfosmeException(EXC_LOGICERROR,"D=%u glossary is not loaded",dlvId_);
        }
        TextList::iterator* iter = hash_->GetPtr(input ? -id : id);
        if (!iter) {
            throw InfosmeException(EXC_NOTFOUND,"D=%u %s text with id=%u is not found",
                                   dlvId_,input ? "input" : "real", id);
        }
        MessageText mt( (*iter)->getText(),
                        returnRealId ? (*iter)->getTextId() : p.getTextId() );
        p.swap(mt);
    }
}


void MessageGlossary::setTexts( const std::vector< std::string >& texts )
{
    if (texts.empty()) return;

    ChangeGuard cg(*this);

    TextList newlist;
    std::auto_ptr<TextHash> newhash;
    if (hash_) {
        newhash.reset(new TextHash(*hash_));
    } else {
        newhash.reset(new TextHash);
    }
    int32_t maxRealId = maxRealId_;

    char fname[100];
    strcat(makeDeliveryPath(fname,dlvId_),"glossary.txt");
    FileGuard nfg;
    nfg.create( (getCS()->getStorePath() + fname + ".tmp").c_str(),0666,true,true);
    if (newhash->Count() > 0) {
        FileGuard ofg;
        ofg.ropen((getCS()->getStorePath() + fname).c_str());
        char readbuf[1024];
        while ( true ) {
            size_t wasread = ofg.read(readbuf,sizeof(readbuf));
            if (!wasread) break;
            nfg.write(readbuf,wasread);
        }
    }

    smsc::core::buffers::TmpBuf<char,2048> textbuf;
    int32_t inputId = 1;
    for ( std::vector<std::string>::const_iterator i = texts.begin();
          i != texts.end(); ++i ) {
        
        TextList::iterator* iter = newhash->GetPtr(-inputId);
        if ( iter && *i == (*iter)->getText() ) {
            // the same text, skip it
            ++inputId;
            continue;
        }
        // new text
        const int32_t newId = ++maxRealId;
        TextList::iterator node = newlist.insert(newlist.end(),Node(i->c_str(),newId));
        newhash->Insert(newId,node);
        if (iter) {
            *iter = node;
        } else {
            newhash->Insert(-inputId,node);
        }
        // writing the record
        int off = sprintf(textbuf.get(),"%u,",inputId);
        textbuf.SetPos(size_t(off));
        escapeText(textbuf,node->getText(),strlen(node->getText()));
        *textbuf.GetCurPtr() = '\n';
        nfg.write(textbuf.get(),textbuf.GetPos()+1);
        ++inputId;
    }
    nfg.close();
    {
        if ( -1 == rename( (getCS()->getStorePath() + fname + ".tmp").c_str(),
                           (getCS()->getStorePath() + fname).c_str()) ) {
            // FIXME: cleanup newList
            throw InfosmeException(EXC_SYSTEM,"D=%u rename('%s')",dlvId_,fname);
        }
        MutexGuard mg(lock_);
        list_.splice(list_.begin(),newlist,newlist.begin(),newlist.end());
        TextHash* tmp = hash_;
        hash_ = newhash.release();
        newhash.reset(tmp);
        maxRealId_ = maxRealId;
    }
}


void MessageGlossary::getTexts( std::vector< std::string >& texts ) const
{
    ChangeGuard cg(*this);
    if (!hash_) return;

    int id;
    TextList::iterator* node;
    texts.reserve(hash_->Count()/2);
    std::string empty("\0",1);
    for ( TextHash::Iterator i(*hash_); i.Next(id,node); ) {
        if (id>=0) continue;
        id = -id;
        if (texts.size() < unsigned(id)) {
            texts.resize(id,empty);
        }
        texts[id-1] = (*node)->getText();
    }

    id = 0;
    for ( std::vector< std::string >::const_iterator i = texts.begin();
          i != texts.end(); ++i ) {
        ++i;
        if ( i->c_str()[0] == '\0' && i->size() > 0 ) {
            throw InfosmeException(EXC_LOGICERROR,"D=%u input index %d is not filled",dlvId_,id);
        }
    }
}


void MessageGlossary::readGlossaryFailed( unsigned           txtId,
                                          const char*        msg )
{
    smsc_log_warn(log_,"D=%u %s at line %u",dlvId_,msg,txtId);
    throw InfosmeException(EXC_BADFILE,"D=%u %s at line %u",dlvId_,msg,txtId);
}

}
}
