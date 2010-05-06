/*
 * ConfigLocks.hpp
 *
 *  Created on: Feb 18, 2010
 *      Author: skv
 */

#ifndef __EYELINE_CLUSTER_CONTROLLER_CONFIGLOCKS_HPP__
#define __EYELINE_CLUSTER_CONTROLLER_CONFIGLOCKS_HPP__

namespace eyeline{
namespace clustercontroller{

enum ConfigType{
  ctMainConfig,//+
  ctProfiles,//+
  ctMsc,//+
  ctRoutes,//+
  ctSme,//+
  ctClosedGroups,//+
  ctAliases,//+
  ctMapLimits,//+
  ctRegions,
  ctResources,//+
  ctReschedule,//+
  ctSnmp,
  ctTimeZones,//+
  ctFraud,//+

  ctConfigsCount
};


}
}

#endif /* CONFIGLOCKS_HPP_ */
