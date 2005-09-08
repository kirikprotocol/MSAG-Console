#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_MESSAGES__
#define __SMSC_INMAN_INTERACTION_MESSAGES__

#include "inman/common/errors.hpp"
#include "inman/common/util.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "inman/interaction/serializer.hpp"

using std::runtime_error;
using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializableObject;

namespace smsc  {
namespace inman {
namespace interaction {

class StartMessage : public SerializableObject
{
public:
	enum { OBJECT_ID = 1 };
protected:
    virtual void load(ObjectBuffer& in)
    {
    }

    virtual void save(ObjectBuffer& out)
    {
    	out << (USHORT_T) OBJECT_ID;
    }

    virtual void run()
    {
    	fprintf( stderr, "Start event received!\n");
    }
};


}
}
}

#endif
