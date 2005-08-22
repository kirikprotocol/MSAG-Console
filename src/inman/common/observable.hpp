#ident "$Id$"

#ifndef __SMSC_INMAN_INTER_OBSERVABLE__
#define __SMSC_INMAN_INTER_OBSERVABLE__

#include <list>
#include <algorithm>
#include <functional>

namespace smsc  {
namespace inman {
namespace common  {

// Этот функтор удаляет переданный ему указатель
// Полезен при работе с алгоритмами STL, например
// std::for_each( listeners.begin(), listeners.end(), DeletePtr() );
// удаляет все обьекты из контейнера
struct DeletePtr : std::unary_function< void*, void > 
{
	result_type operator( ) ( argument_type ptr )
   	{
      		delete ptr;
   	}
};

// Два следующих класса помогают создавать источники и обработчики событий
// Пример использования:
// class EventReceiver
// {
//	    virtual void zeroParams();
//		virtual void oneParam(int param1)
//		virtual void twoParams(int param1, const char* param2)
//		...
// };
//
// class EventSender : ObservableT< EventReceiver >
// {
//		void sendEvent()
//		{
//			notify0( &EventReceiver::zeroParams );
//			notify1<int>( &EventReceiver::oneParam, 1242 );
//			notify2<int, char*>( &EventReceiver::twoParams, 22432, "cool!" );
//		}
// }

template < class Listener >
class ObservableT
{

public:

	typedef std::list<Listener*> ListenerList;

	ObservableT()
	{
	}

	~ObservableT()
	{
		removeAllListeners();
	}

	void addListener(Listener* pListener)
	{
		listeners.push_back( pListener );
	}

	void removeListener(Listener* pListener)
	{
		listeners.remove( pListener );
	}

	void removeAllListeners()
	{
		std::for_each( listeners.begin(), listeners.end(), DeletePtr() );
		listeners.clear();
	}

protected:

	void notify0(void (Listener::*method)())
	{
		for( typename ListenerList::iterator it = listeners.begin();
		it != listeners.end(); it++)
	 	{
			 Listener* ptr = *it;
			 (*ptr.*method)();
		}
	}

	template <typename param1_t>
	void notify1(void (Listener::*method)(param1_t), param1_t param1)
	{
		for( typename ListenerList::iterator it = listeners.begin();
		it != listeners.end(); it++)
	 	{
			 Listener* ptr = *it;
			 (*ptr.*method)( param1 );
		}
	}

	template <typename param1_t, typename param2_t>
	void notify2(void (Listener::*method)(param1_t, param2_t), param1_t param1, param2_t param2)
	{
		for( typename ListenerList::iterator it = listeners.begin();
		it != listeners.end(); it++)
	 	{
			 Listener* ptr = *it;
			 (*ptr.*method)( param1, param2 );
		}
	}


	ListenerList listeners;
};

}
}
}

#endif
