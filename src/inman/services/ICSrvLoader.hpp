/* ************************************************************************** *
 * INMan configurable service shared library loader
 * ************************************************************************** */
#ifndef __INMAN_ICSERVICE_LOADER_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICSERVICE_LOADER_HPP__

#include "util/config/ConfigException.h"

#include "inman/services/ICSrvDefs.hpp"

namespace smsc  {
namespace inman {

using smsc::util::config::ConfigException;

//This is the Inman Configurable Service dynamic library entry point
extern "C" ICSProducerAC * loadupICService(void);

class ICSrvLoader {
public:
    static ICSProducerAC * LoadICS(const char * dl_name) throw(ConfigException);

protected:
    void* operator new(size_t);
    ICSrvLoader() { }
    ~ICSrvLoader() { }
};


} //inman
} //smsc

#endif /* __INMAN_ICSERVICE_LOADER_HPP__ */
