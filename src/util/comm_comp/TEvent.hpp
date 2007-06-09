// -*-C++-*-
#ifndef TEVENT_H
# define TEVENT_H

# include <iostream>
# include <stdio.h>
# include <list>

namespace smsc {
namespace util {
namespace comm_comp {

/*
** Type Message desing pattern with some improvement.
** For pattern description see John Vlissides "Pattern hatching".
*/
class AbstractEvent {
public:
  virtual ~AbstractEvent() {}
  virtual void Notify (const std::string& connId) const = 0;
};

template <class T> class TEvent : public AbstractEvent {
public:
  class Handler {
  public:
    Handler() { 
      TEvent<T>::Register(this); 
    }
    ~Handler() { 
      TEvent<T>::Unregister(this); 
    }
    virtual void handleEvent(const T *t, const std::string& connId) = 0;
  };

  typedef std::list<Handler*> HandlerList;

  static void Register (Handler* aHandler) {
    registry.push_back(aHandler);
  }

  static void Unregister (Handler* aHandler) {
    registry.remove(aHandler);
  }

  static void Notify (const TEvent<T>* t, const std::string& connId) {
    typename HandlerList::iterator i;
    for (i = registry.begin(); i != registry.end(); i++) {
      (*i)->handleEvent((T*) t, connId);
    }
  }

  virtual void Notify (const std::string& connId) const { T::Notify(this, connId); }

private:
  static HandlerList registry;
};

}}}
#endif
