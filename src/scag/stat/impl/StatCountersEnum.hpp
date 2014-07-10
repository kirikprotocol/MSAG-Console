/*
 * StatCountersEnum.hpp
 *
 *  Created on: May 15, 2014
 *      Author: ksv
 */

#ifndef STATCOUNTERSENUM_HPP_
#define STATCOUNTERSENUM_HPP_


namespace scag2 {
namespace stat {

  namespace Counters
  {
    typedef enum
    {
      cntAccepted,
      cntRejected,
      cntDelivered,
      cntGw_Rejected,
      cntFailed,

      cntRecieptOk,
      cntRecieptFailed,
      cntSmppSize
    } SmppStatCounter;

    typedef enum
    {
      httpRequest,
      httpRequestRejected,
      httpResponse,
      httpResponseRejected,
      httpDelivered,
      httpFailed,
      cntHttpSize
    } HttpStatCounter;
  }

}}

#endif /* STATCOUNTERSENUM_HPP_ */
