#ifndef __INMAN_USS_EMULATOR_REQUESTPROCESSINGRULES_HPP__
# define __INMAN_USS_EMULATOR_REQUESTPROCESSINGRULES_HPP__

# include <util/config/ConfigView.h>

namespace smsc  {
namespace inman {
namespace uss  {

struct RequestProcessingRules
{
  void init(const util::config::ConfigView& rulesConfig) {
    try {
      mandatoryDelay = rulesConfig.getInt("mandatoryDelay");
    } catch (util::config::ConfigException & ex) {
      mandatoryDelay = 300;
    }
    try {
      oneSecondDelayProbability = rulesConfig.getInt("oneSecondDelayProbability");
    } catch (util::config::ConfigException & ex) {
      oneSecondDelayProbability = 10;
    }
    try {
      twoSecondsDelayProbability = rulesConfig.getInt("twoSecondsDelayProbability");
    } catch (util::config::ConfigException & ex) {
      twoSecondsDelayProbability = 10;
    }
    try {
      fourSecondsDelayProbability = rulesConfig.getInt("fourSecondsDelayProbability");
    } catch (util::config::ConfigException & ex) {
      fourSecondsDelayProbability = 10;
    }
    try {
      rejectProbability = rulesConfig.getInt("rejectProbability");
    } catch (util::config::ConfigException & ex) {
      rejectProbability = 10;
    }
    try {
      nonOkStatusProbability = rulesConfig.getInt("nonOkStatusProbability");
    } catch (util::config::ConfigException & ex) {
      nonOkStatusProbability = 10;
    }

    if ( oneSecondDelayProbability + twoSecondsDelayProbability +
         fourSecondsDelayProbability + rejectProbability + nonOkStatusProbability > 100 )
      throw util::config::ConfigException("total probability value is greater than 100");
  }

  std::string toString() const {
    char strBuf[1024];
    snprintf(strBuf,sizeof(strBuf), "mandatoryDelay=%d,oneSecondDelayProbability=%d,twoSecondsDelayProbability=%d,fourSecondsDelayProbability=%d,rejectProbability=%d,nonOkStatusProbability=%d", mandatoryDelay, oneSecondDelayProbability, twoSecondsDelayProbability, fourSecondsDelayProbability, rejectProbability, nonOkStatusProbability);
    return strBuf;
  }

  unsigned mandatoryDelay;
  unsigned oneSecondDelayProbability, twoSecondsDelayProbability, fourSecondsDelayProbability;
  unsigned rejectProbability, nonOkStatusProbability;
};

}}}

#endif
