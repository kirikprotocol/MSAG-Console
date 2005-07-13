/*
 * Copyright 1999-2000,2004 The Apache Software Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Log$
 * Revision 1.1  2005/07/08 08:17:51  loomox
 *
 * Emulation RulesEngine with XML-parsing
 * Added "if","then","choose" action whithout parsing params
 *
 * Revision 1.4  2004/09/08 13:55:33  peiyongz
 * Apache License Version 2.0
 *
 * Revision 1.3  2003/05/30 09:36:36  gareth
 * Use new macros for iostream.h and std:: issues.
 *
 * Revision 1.2  2003/02/05 18:53:23  tng
 * [Bug 11915] Utility for freeing memory.
 *
 * Revision 1.1  2000/08/02 19:16:14  jpolast
 * initial checkin of SAX2Print
 *
 *
 */

// ---------------------------------------------------------------------------
//  Includes for all the program files to see
// ---------------------------------------------------------------------------
#include <string.h>
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif
#include <stdlib.h>

#include    <xercesc/sax2/DefaultHandler.hpp>
#include    <xercesc/framework/XMLFormatter.hpp>
XERCES_CPP_NAMESPACE_USE



