#ifndef __SMPPDMPLX_CONFIGURATION_HPP__
# define __SMPPDMPLX_CONFIGURATION_HPP__

# include <util/Singleton.hpp>
# include <sys/types.h>
# include <netinet/in.h>
# include <map>

namespace smpp_dmplx {

struct Configuration : public smsc::util::Singleton<Configuration>
{
  in_port_t listenPort;
  std::string smscHost;
  in_port_t smscPort;
  time_t    inactivityPeriod;
  time_t    selectTimeOut;
  typedef std::map<std::string, std::string> systemid_to_passwd_map_t;
  systemid_to_passwd_map_t sysid_Passwd_map;
};

}

#endif
