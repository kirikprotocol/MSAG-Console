#ifndef __SMSC_INFO_SME_INFOSME_TASKS_STAT_SEARCHCRITERION_HPP__
# define __SMSC_INFO_SME_INFOSME_TASKS_STAT_SEARCHCRITERION_HPP__ 1

# include <time.h>
# include <string>

namespace smsc{ namespace infosme
{

class InfoSme_Tasks_Stat_SearchCriterion
{
public:
  InfoSme_Tasks_Stat_SearchCriterion() : _isSetTaskId(false), _isSetStartPeriod(false), _isSetEndPeriod(false) {}
  void setTaskId(const std::string& taskId) {
    _isSetTaskId = true;
    _taskId = taskId;
  }
  bool isSetTaskId() const { return _isSetTaskId; }
  const std::string& getTaskId() const { return _taskId; }

  void setStartPeriod(uint32_t startPeriod) {
    _isSetStartPeriod = true;
    _startPeriod = startPeriod;
  }
  bool isSetStartPeriod() const { return _isSetStartPeriod; }
  uint32_t getStartPeriod() const { return _startPeriod; }

  void setEndPeriod(uint32_t endPeriod) {
    _isSetEndPeriod = true;
    _endPeriod = endPeriod;
  }
  bool isSetEndPeriod() const { return _isSetEndPeriod; }
  uint32_t getEndPeriod() const { return _endPeriod; }
private:
  bool _isSetTaskId;
  std::string _taskId;

  bool _isSetStartPeriod;
  uint32_t _startPeriod;

  bool _isSetEndPeriod;
  uint32_t _endPeriod;
};

}
}
#endif
