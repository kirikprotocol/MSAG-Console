#ifndef _SCAG_COUNTER_ACTIONTABLE_H
#define _SCAG_COUNTER_ACTIONTABLE_H

#include <list>
#include "Counter.h"
#include "Observer.h"

namespace scag2 {
namespace counter {

class ActionTable;

struct ActionList
{
    friend class ActionTable;
public:
    ActionList() : capacity(0), size(0), list(0) {}
    ActionList( const ActionList& l ) :
    capacity(l.size), size(l.size), list(0) {
        if (size>0) {
            list = new ActionLimit[size];
            memcpy(list,l.list,size*sizeof(ActionLimit));
        }
    }
    ActionList& operator = ( const ActionList& l ) {
        if (this != &l) {
            if (l.size>size) {
                delete [] list;
                capacity = l.size;
                list = new ActionLimit[capacity];
            }
            size = l.size;
            if (size>0) {
                memcpy(list,l.list,size*sizeof(ActionLimit));
            }
        }
        return *this;
    }

    ~ActionList() {
        if (list) delete[] list;
    }

    /// this method should be used only 
    void push_back( const ActionLimit& a ) {
        if ( size >= capacity ) {
            capacity = size + 8;
            ActionLimit* newlist = new ActionLimit[capacity];
            memcpy(newlist,list,size*sizeof(ActionLimit));
            delete[] list;
            list = newlist;
        }
        list[size++] = a;
    }

private:
    size_t        capacity;
    size_t        size;      // number of actions
    ActionLimit*  list;      // owned, delete []
};


class ActionTable : public Observer
{
protected:
    static smsc::logger::Logger* log_;
    virtual ~ActionTable();
public:
    ActionTable( ActionList* list = 0 );
    virtual void modified( const char*  cname,
                           CntSeverity& sev,
                           int64_t      value,
                           unsigned     maxval );

    /// NOTE: do not fiddle with this method
    virtual void ref(bool add);

    /// setting a new actions
    void setNewActions( ActionList* newlist );

private:
    // void notify( Counter& c, int64_t value, const ActionParams& params );

private:
    smsc::core::synchronization::Mutex    lock_;
    unsigned                              ref_;
    smsc::core::synchronization::Mutex    actlock_;
    ActionList*                           actions_; // owned
    typedef std::pair<time_t,ActionList*> OldList;
    std::list< OldList >                  oldlists_;
};

}
}


#endif /* !_SCAG_COUNTER_ACTIONTABLE_H */
