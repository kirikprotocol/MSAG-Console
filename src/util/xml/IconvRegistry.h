// 
// File:   IconvRegistry.h
// Author: igork
//
// Created on 1 Март 2004 г., 15:59
//

#ifndef _IconvRegistry_H
#define	_IconvRegistry_H

#define LIBICONV_PLUG EMPTY
#include <iconv.h>

namespace smsc {
namespace util {
namespace xml {

iconv_t getIconv(const char * const from, const char * const to) throw ();
void clearIconvs() throw();

}
}
}

#endif	/* _IconvRegistry_H */

