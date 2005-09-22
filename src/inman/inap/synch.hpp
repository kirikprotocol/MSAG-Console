#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_SYNC__
#define __SMSC_INMAN_INAP_SYNC__

#include "core/synchronization/Mutex.hpp"
using smsc::core::synchronization::Mutex;


namespace smsc  {
namespace inman {
namespace inap  {

class Synch : public Mutex
{
	protected:
		Synch() : Mutex()
		{
			Lock();
		}

	public:
		static Synch* getInstance()
		{
			static Synch instance;
			return &instance;
		}
};

}
}
}

#endif
