/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision% */
/*                                                                   */
/* ss7cp.h,v */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech AB 1998                               */
/*                                                                   */
/* The copyright to the computer program herein is the property of   */
/* Ericsson Infotech AB. The program may be used and/or  copied only */
/* with the written permission from Ericsson Infotech  AB or in the  */
/* accordance with the terms and conditions stipulated in the        */
/* agreement/contract under which the program has been supplied.     */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                 (Rev:    )                       */
/* 32/190 55-CAA 201 29 Ux   (<revision>)                           */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: ss7cp.h,v 1.7 2000/05/26 14:21:49 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/N/B Dan Liljemark                                             */
/*                                                                   */
/* Purpose:                                                          */
/*   This file is the general header file for Common Parts           */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 00-98XXXX  EIN/N/B Dan Liljemark                                  */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/

/* Recursion semaphore */
#ifndef __SS7CP_H__
#define __SS7CP_H__

/*********************************************************************/
/****                                                             ****/
/****                   I  N  C  L  U  D  E  S                    ****/
/****                                                             ****/
/*********************************************************************/
#include "ss7osdpn.h"
#include "portss7.h"
#include "ss7msg.h"
#include "ss7tmc.h"
#include "ss7log.h"
#include "ss7util.h"
#ifdef EINSS7_CORE
  #include "ss7autoconfig.h"
#endif /* EINSS7_CORE */

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif

/*********************************************************************/
/****                                                             ****/
/****                  C  O  N  S  T  A  N  T  S                  ****/
/****                                                             ****/
/*********************************************************************/

/*********************************************************************/
/****                                                             ****/
/****                    G  L  O  B  A  L  S                      ****/
/****                                                             ****/
/*********************************************************************/

/*********************************************************************/
/****                                                             ****/
/****                      L  O  C  A  L  S                       ****/
/****                                                             ****/
/*********************************************************************/

/*********************************************************************/
/****                                                             ****/
/****                  S  T  R  U  C  T  U  R  E  S               ****/
/****                                                             ****/
/*********************************************************************/

/*********************************************************************/
/****                                                             ****/
/****                     T  Y  P  E  D  E  F  S                  ****/
/****                                                             ****/
/*********************************************************************/

/*********************************************************************/
/****                                                             ****/
/****                 P  R  O  T  O  T  Y  P  E  S                ****/
/****                                                             ****/
/*********************************************************************/

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif


#endif /* __SS7CP_H__ */
