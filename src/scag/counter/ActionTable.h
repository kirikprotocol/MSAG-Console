#ifndef _SCAG_COUNTER_ACTIONTABLE_H
#define _SCAG_COUNTER_ACTIONTABLE_H

#include <list>
#include "Counter.h"
#include "Observer.h"

namespace scag2 {
namespace counter {

struct ActionList
{
public:
    ActionList() : capacity(0), size(0), list(0) {}

    ~ActionList() {
        delete[] list;
    }

    /// this method should be used only 
    void push_back( const ActionParams& a ) {
        if ( size >= capacity ) {
            capacity = size + 8;
            ActionParams* newlist = new ActionParams[capacity];
            memcpy(newlist,list,size);
            delete[] list;
            list = newlist;
        }
        list[size++] = a;
    }

private:
    size_t  capacity;
public:
    size_t        size;      // number of actions
    ActionParams* list;      // owned, delete []
};


class ActionTable : public Observer
{
protected:
    static smsc::logger::Logger* log_;
    virtual ~ActionTable();
public:
    ActionTable();
    virtual void modified( Counter& counter, int64_t value );

    /// NOTE: do not fiddle with this method
    virtual void ref(bool add);

    /// setting a new actions
    void setNewActions( ActionList* newlist );

private:
    void notify( Counter& c, int64_t value, const ActionParams& params );

private:
    smsc::core::synchronization::Mutex    lock_;
    unsigned                              ref_;
    smsc::core::synchronization::Mutex    actlock_;
    ActionList*                           actions_; // owned[]
    typedef std::pair<time_t,ActionList*> OldList;
    std::list< OldList >                  oldlists_;
};

}
}


#endif /* !_SCAG_COUNTER_ACTIONTABLE_H */
