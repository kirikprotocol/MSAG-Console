/*
 * XercesMigration.h
 *
 *  Created on: Oct 2, 2013
 *      Author: ksv
 */

#ifndef XERCESMIGRATION_H_
#define XERCESMIGRATION_H_

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_USE

#if XERCES_VERSION_MAJOR > 2
typedef XMLSize_t XERCES_UINT;
typedef XMLFileLoc XERCES_FILELOC;
#else
typedef unsigned int XERCES_UINT;
typedef XMLSSize_t XERCES_FILELOC;
#endif


#endif /* XERCESMIGRATION_H_ */
