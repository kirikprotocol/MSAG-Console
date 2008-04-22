#include "DateTimeAction.h"
#include <time.h>


namespace scag { namespace re { namespace actions {

const char* CURRENT_DATETIME_ACTION = "datetime:current";
const char* ADD_DATETIME_ACTION     = "datetime:add";
const char* DEC_DATETIME_ACTION     = "datetime:dec";
const char* CHANGE_DATETIME_ACTION  = "datetime:change";
const char* DATETIME_ACTION         = "DateTimeAction";
const char* DATE_PROPERTY           = "date";
const char* TIME_PROPERTY           = "time";
const char* DATETIME_PROPERTY       = "datetime";
const char* DST_TRUE                = "TRUE";
const char* DST_FALSE               = "FALSE";

const char* DATETIME_FORMAT = "%d.%m.%Y %H:%M:%S";
const char* DATE_FORMAT = "%d.%m.%Y";
const char* TIME_FORMAT = "%H:%M:%S";

const size_t MIN_YEAR = 1900;
const size_t MAX_YEAR = 2100;


enum DateProperties {
  DATE_YEAR,
  DATE_MONTH,
  DATE_MDAY,
  DATE_YDAY,
  DATE_WDAY
};

static const char* dateProperties[] = {
  "year",
  "month",
  "day",
  "yday",
  "wday"
};

const size_t DATE_PROPERTIES_NUMBER = 5;

enum TimeProperties {
  TIME_HOUR,
  TIME_MIN,
  TIME_SEC,
  TIME_TIMEZONE,
  TIME_ISDST
};

static const char* timeProperties[] = {
  "hour",
  "min",
  "sec",
  "timezone",
  "isdst"
};

const size_t TIME_PROPERTIES_NUMBER = 5;

const size_t DATETIME_BUF_SIZE = 32;

bool DateTimeAction::addProperty(const SectionParams &params, PropertyObject &propertyObject, 
                                 const char* propertyName, bool isInt, bool isRequired, bool readOnly){
  string strParameter;
  bool exist = false;
  ActionProperty actionProperty;
  FieldType ft = CheckParameter(params, propertyObject, DATETIME_ACTION, propertyName, isRequired,
                                 readOnly, strParameter, exist);
  if (!exist) {
    return false;
  }
  actionProperty.setType(ft);
  if (!actionProperty.setValue(strParameter, isInt)) {
    throw SCAGException("%s '%s' : '%s' parameter shuold be integer %s='%s'", DATETIME_ACTION,
                        getActionName(), propertyName, propertyName, strParameter.c_str());
  }
  properties.Insert(propertyName, actionProperty);
  return true;
}

void DateTimeAction::intiDateModifierAction(const SectionParams &params,
                                                        PropertyObject& propertyObject){
  string strParameter;
  bool exist = false;

  FieldType ft = CheckParameter(params, propertyObject, DATETIME_ACTION, DATE_PROPERTY, false,
                                 false, strParameter, exist);
  if (exist) {
    dateTimeProperty = new DateProperty(ft, strParameter);
    for (int i = 0; i < DATE_PROPERTIES_NUMBER - 1; ++i) {
      addProperty(params, propertyObject, dateProperties[i], true, false, true);
    }
  }

  ft = CheckParameter(params, propertyObject, DATETIME_ACTION, TIME_PROPERTY, false,
                      false, strParameter, exist);
  if (exist) {
    if (dateTimeProperty) {
      throw SCAGException("%s '%s' action should has only one of 'date' 'time' 'datetime' properties",
                           DATETIME_ACTION, getActionName());
    }
    dateTimeProperty = new TimeProperty(ft, strParameter);
    for (int i = 0; i < TIME_PROPERTIES_NUMBER - 2; ++i) {
      addProperty(params, propertyObject, timeProperties[i], true, false, true);
    }
    //getDst(params, propertyObject);
  }

  ft = CheckParameter(params, propertyObject, DATETIME_ACTION, DATETIME_PROPERTY, false,
                      false, strParameter, exist);
  if (exist) {
    if (dateTimeProperty) {
      throw SCAGException("%s '%s' action should has only one of 'date' 'time' 'datetime' properties",
                           DATETIME_ACTION, getActionName());
    }
    dateTimeProperty = new DateTimeProperty(ft, strParameter);
    for (int i = 0; i < DATE_PROPERTIES_NUMBER - 1; ++i) {
      addProperty(params, propertyObject, dateProperties[i], true, false, true);
    }
    for (int i = 0; i < TIME_PROPERTIES_NUMBER - 2; ++i) {
      addProperty(params, propertyObject, timeProperties[i], true, false, true);
    }
    //getDst(params, propertyObject);
  }

  if (!dateTimeProperty) {
    throw SCAGException("%s '%s' action should has at least one of 'date' 'time' 'datetime' properties",
                         DATETIME_ACTION, getActionName());
  }
}

bool DateTimeAction::getDst(const SectionParams &params, PropertyObject &propertyObject) {
  string value;
  bool exist = false;
  ActionProperty actionProperty;
  FieldType ft = CheckParameter(params, propertyObject, DATETIME_ACTION, timeProperties[TIME_ISDST],
                                false, true, value, exist);
  if (!exist) {
    return false;
  }
  if (value.compare(DST_TRUE) == 0) {
    return isDst = true;
  }
  if (value.compare(DST_FALSE) == 0) {
    return isDst = false;
  }
  throw SCAGException("%s '%s' : '%s' parameter shuold have TRUE or FALSE value %s='%s'", DATETIME_ACTION,
                        getActionName(), timeProperties[TIME_ISDST], timeProperties[TIME_ISDST], value.c_str());
}

IParserHandler * DateTimeAction::StartXMLSubSection(const std::string &name, const SectionParams &params, const ActionFactory &factory) {
  throw SCAGException("%s '%s' cannot include child objects", getActionName(),DATETIME_ACTION);
}

bool DateTimeAction::FinishXMLSubSection(const std::string &name) {
  return  true;
}

void CurrentDateTimeAction::init(const SectionParams &params, PropertyObject propertyObject) {
  smsc_log_debug(logger, "Init 'datetime:current' action");
  addProperty(params, propertyObject, DATE_PROPERTY, false, false, false);
  addProperty(params, propertyObject,TIME_PROPERTY, false, false, false);
  addProperty(params, propertyObject,DATETIME_PROPERTY, false, false, false);
  for (int i = 0; i < DATE_PROPERTIES_NUMBER; ++i) {
    addProperty(params, propertyObject, dateProperties[i], true, false, false);
  }
  for (int i = 0; i < TIME_PROPERTIES_NUMBER - 2; ++i) {
    addProperty(params, propertyObject, timeProperties[i], true, false, false);
  }
  addProperty(params, propertyObject, timeProperties[TIME_TIMEZONE], true, false, true);
  getDst(params, propertyObject);
}

bool CurrentDateTimeAction::checkTimeZone(int timeZone) {
  if (timeZone >= -12 && timeZone <= 13) {
    return true;
  } else {
    smsc_log_warn(logger, "%s 'datetime:current' : error time zone %d", DATETIME_ACTION, timeZone);
    return false;
  }
}

bool CurrentDateTimeAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action 'datetime:current'");
  time_t t = time(NULL);
  struct tm time_struct;
  gmtime_r(&t, &time_struct);
  ActionProperty* p = properties.GetPtr(timeProperties[TIME_TIMEZONE]);
  int time_zone = 0;
  if (p && (time_zone = p->getIntValue(context)) != 0 && checkTimeZone(time_zone)) {
    smsc_log_debug(logger, "Run 'datetime:current' action: timezone = %d", time_zone);
    t += time_zone * 3600;
    gmtime_r(&t, &time_struct);
    if (isDst) {
      mktime(&time_struct);
    }
  }
  p = properties.GetPtr(DATETIME_PROPERTY);
  if (p) {
    char buf[DATETIME_BUF_SIZE];
    strftime(buf, DATETIME_BUF_SIZE, DATETIME_FORMAT, &time_struct);
    p->setStrValue(buf, context);
  }
  p = properties.GetPtr(DATE_PROPERTY);
  if (p) {
    char buf[DATETIME_BUF_SIZE];
    strftime(buf, DATETIME_BUF_SIZE, DATE_FORMAT, &time_struct);
    p->setStrValue(buf, context);
  }
  p = properties.GetPtr(TIME_PROPERTY);
  if (p) {
    char buf[DATETIME_BUF_SIZE];
    strftime(buf, DATETIME_BUF_SIZE, TIME_FORMAT, &time_struct);
    p->setStrValue(buf, context);
  }
  p = properties.GetPtr(timeProperties[TIME_HOUR]);
  if (p) {
    p->setIntValue(time_struct.tm_hour, context);
  }
  p = properties.GetPtr(timeProperties[TIME_MIN]);
  if (p) {
    p->setIntValue(time_struct.tm_min, context);
  }
  p = properties.GetPtr(timeProperties[TIME_SEC]);
  if (p) {
    p->setIntValue(time_struct.tm_sec, context);
  }
  p = properties.GetPtr(dateProperties[DATE_YEAR]);
  if (p) {
    p->setIntValue(time_struct.tm_year + 1900, context);
  }
  p = properties.GetPtr(dateProperties[DATE_MONTH]);
  if (p) {
    p->setIntValue(time_struct.tm_mon + 1, context);
  }
  p = properties.GetPtr(dateProperties[DATE_MDAY]);
  if (p) {
    p->setIntValue(time_struct.tm_mday, context);
  }
  p = properties.GetPtr(dateProperties[DATE_WDAY]);
  if (p) {
    size_t wday = time_struct.tm_wday == 0 ? 1 : time_struct.tm_wday;
    p->setIntValue(wday, context);
  }
  p = properties.GetPtr(dateProperties[DATE_YDAY]);
  if (p) {
    p->setIntValue(time_struct.tm_yday, context);
  }
  return true;
}

const char* CurrentDateTimeAction::getActionName() const {
  return CURRENT_DATETIME_ACTION;
}

const char* ChangeDateTimeAction::getActionName() const {
  return CHANGE_DATETIME_ACTION;
}

void ChangeDateTimeAction::init(const SectionParams &params, PropertyObject propertyObject) {
  intiDateModifierAction(params, propertyObject);
}

bool ChangeDateTimeAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'", getActionName());
  if (dateTimeProperty) {
    dateTimeProperty->change(context, properties, getActionName());
  }
  return true;
}
const char* AddDateTimeAction::getActionName() const {
  return ADD_DATETIME_ACTION;
}

void AddDateTimeAction::init(const SectionParams &params, PropertyObject propertyObject) {
  intiDateModifierAction(params, propertyObject);
}

bool AddDateTimeAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'", getActionName());
  if (dateTimeProperty) {
    dateTimeProperty->add(context, properties, getActionName(), sign);
  }
  return true;
}

const char* DecDateTimeAction::getActionName() const {
  return DEC_DATETIME_ACTION;
}

void DecDateTimeAction::init(const SectionParams &params, PropertyObject propertyObject) {
  intiDateModifierAction(params, propertyObject);
}

bool DecDateTimeAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'", getActionName());
  if (dateTimeProperty) {
    dateTimeProperty->add(context, properties, getActionName(), sign);
  }
  return true;
}

bool DateTimeModifier::checkYear(int year, const char* actionName) {
  if (year > MIN_YEAR && year < MAX_YEAR) {
    return true;
  } else {
    smsc_log_warn(logger, "%s '%s' action : error year value %d. Should be: %d < year < %d",
                  DATETIME_ACTION, actionName, year, MIN_YEAR, MAX_YEAR);
    return false;
  }
}

bool DateProperty::getOldDateTime(ActionContext &context, const char* actionName, struct tm& dateTime) {
  string dateTimeStr = property.getStrValue(context);
  if (dateTimeStr.empty()) {
    smsc_log_warn(logger, "%s '%s' action : date property is empty",
                  DATETIME_ACTION, actionName);
    return false;
  }
  int old_day = 0;
  int old_mon = 0;
  int old_year = 0;
  int scanned = sscanf(dateTimeStr.c_str(), "%02d.%02d.%04d", &old_day, &old_mon, &old_year);
  if (scanned != 3) {
    smsc_log_warn(logger, "%s '%s' action : error date format '%s'. date format should be 'dd.mm.yyyy'",
                  DATETIME_ACTION, actionName, dateTimeStr.c_str());
    return false;
  }
  if (!checkYear(old_year, actionName)) {
    return false;
  }
  memset(&dateTime, 0, sizeof(tm));
  dateTime.tm_year = old_year - 1900;
  dateTime.tm_mon = old_mon - 1;
  dateTime.tm_mday = old_day;
  return true;
}

bool DateProperty::add(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName, int sign) {
  struct tm date;
  if (!getOldDateTime(context, actionName, date)) {
    return false;
  }
  ActionProperty* p = properties.GetPtr(dateProperties[DATE_YEAR]);
  if (p) {
    date.tm_year += p->getIntValue(context) * sign;
  }
  p = properties.GetPtr(dateProperties[DATE_MONTH]);
  if (p) {
    date.tm_mon += p->getIntValue(context) * sign;
  }
  p = properties.GetPtr(dateProperties[DATE_MDAY]);
  if (p) {
    date.tm_mday += p->getIntValue(context) * sign;
  }
  setDateTime(actionName, DATE_FORMAT, date, context);
  return true;
}

bool DateProperty::change(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName) {
  struct tm date;
  if (!getOldDateTime(context, actionName, date)) {
    return false;
  }
  ActionProperty* p = properties.GetPtr(dateProperties[DATE_YEAR]);
  if (p) {
    int new_year = p->getIntValue(context);
    if (checkYear(new_year, actionName)) {
      date.tm_year = new_year - 1900;
    }
  }
  p = properties.GetPtr(dateProperties[DATE_MONTH]);
  if (p) {
    int new_mon = p->getIntValue(context);
    if (new_mon > 0) {
      date.tm_mon = new_mon - 1;
    } else {
      smsc_log_warn(logger, "%s '%s' action : error month new value %d",
                    DATETIME_ACTION, actionName, new_mon);
    }
  }

  p = properties.GetPtr(dateProperties[DATE_MDAY]);
  if (p) {
    int new_day = p->getIntValue(context);
    if (new_day > 0) {
      date.tm_mday = new_day;
    } else {
      smsc_log_warn(logger, "%s '%s' action : error day new value %d",
                    DATETIME_ACTION, actionName, new_day);
    }
  }
  setDateTime(actionName, DATE_FORMAT, date, context);
  return true;
}

bool TimeProperty::getOldDateTime(ActionContext &context, const char* actionName, struct tm& dateTime) {
  string dateTimeStr = property.getStrValue(context);
  if (dateTimeStr.empty()) {
    smsc_log_warn(logger, "%s '%s' action : time property is empty",
                  DATETIME_ACTION, actionName);
    //TODO set old date to current date if old date empty
    return false;
  }
  memset(&dateTime, 0, sizeof(tm));
  dateTime.tm_year = 70;
  int scanned = sscanf(dateTimeStr.c_str(), "%02d:%02d:%02d", 
                       &dateTime.tm_hour, &dateTime.tm_min, &dateTime.tm_sec);
  if (scanned != 3) {
    smsc_log_warn(logger, "%s '%s' action : error time format '%s'. time format should be 'hh:mm:ss'",
                  DATETIME_ACTION, actionName,  dateTimeStr.c_str());
    return false;
  }
  return true;
}

bool DateTimeModifier::setDateTime(const char* actionName, const char* datetimeFormat,
                                      struct tm& dateTime, ActionContext& context) {
  if (mktime(&dateTime) == -1) {
    char buf[DATETIME_BUF_SIZE];
    strftime(buf, DATETIME_BUF_SIZE, datetimeFormat, &dateTime);
    smsc_log_warn(logger, "%s '%s' action : error time format. mktime can't normalize struct tm. time: '%s'",
                  DATETIME_ACTION, actionName, buf);
    return false;
  }
  if (dateTime.tm_isdst == 1) {
    dateTime.tm_hour -= 1;
    if (mktime(&dateTime) == -1) {
      smsc_log_warn(logger, "%s '%s' action : error time format. mktime can't normalize struct tm",
                    DATETIME_ACTION, actionName);
      return false;
    }
  }
  if (!checkYear(dateTime.tm_year + 1900, actionName)) {              
    return false;
  }
  char buf[DATETIME_BUF_SIZE];
  strftime(buf, DATETIME_BUF_SIZE, datetimeFormat, &dateTime);
  property.setStrValue(buf, context);
  return true;
}

bool TimeProperty::change(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName) {
  struct tm time_struct;
  if (!getOldDateTime(context, actionName, time_struct)) {
    return false;
  }

  ActionProperty* p = properties.GetPtr(timeProperties[TIME_HOUR]);
  if (p) {
    time_struct.tm_hour = p->getIntValue(context);
  }
  p = properties.GetPtr(timeProperties[TIME_MIN]);
  if (p) {
    time_struct.tm_min = p->getIntValue(context);
  }
  p = properties.GetPtr(timeProperties[TIME_SEC]);
  if (p) {
    time_struct.tm_sec = p->getIntValue(context);
  }
  setDateTime(actionName, TIME_FORMAT, time_struct, context);
  return true;
}

bool TimeProperty::add(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName, int sign) {
  struct tm time_struct;
  if (!getOldDateTime(context, actionName, time_struct)) {
    return false;
  }
  ActionProperty* p = properties.GetPtr(timeProperties[TIME_HOUR]);
  if (p) {
    time_struct.tm_hour += p->getIntValue(context) * sign;
  }
  p = properties.GetPtr(timeProperties[TIME_MIN]);
  if (p) {
    time_struct.tm_min += p->getIntValue(context) * sign;
  }
  p = properties.GetPtr(timeProperties[TIME_SEC]);
  if (p) {
    time_struct.tm_sec += p->getIntValue(context) * sign;
  }
  setDateTime(actionName, TIME_FORMAT, time_struct, context);
  return true;
}

bool DateTimeProperty::getOldDateTime(ActionContext &context, const char* actionName, struct tm& dateTime) {
  string dateTimeStr = property.getStrValue(context);
  if (dateTimeStr.empty()) {
    smsc_log_warn(logger, "%s '%s' action : datetime property is empty",
                  DATETIME_ACTION, actionName);
    return false;
  }
  memset(&dateTime, 0, sizeof(tm));
  int old_day = 0;
  int old_mon = 0;
  int old_year = 0;
  int scanned = sscanf(dateTimeStr.c_str(), "%02d.%02d.%04d %02d:%02d:%02d", &old_day, &old_mon, &old_year,
                        &dateTime.tm_hour, &dateTime.tm_min, &dateTime.tm_sec);
  if (scanned != 6) {
    smsc_log_warn(logger, "%s '%s' action : error datetime format '%s'. datetime format should be 'dd.mm.yyyy hh:mm:ss'",
                  DATETIME_ACTION, actionName, dateTimeStr.c_str());
    return false;
  }
  if (!checkYear(old_year, actionName)) {
    return false;
  }
  dateTime.tm_year = old_year - 1900;
  dateTime.tm_mon = old_mon - 1;
  dateTime.tm_mday = old_day;
  return true;
}

bool DateTimeProperty::change(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName) {
  struct tm dateTime;
  if (!getOldDateTime(context, actionName, dateTime)) {
    return false;
  }
  ActionProperty* p = properties.GetPtr(timeProperties[TIME_HOUR]);
  if (p) {
    dateTime.tm_hour = p->getIntValue(context);
  }
  p = properties.GetPtr(timeProperties[TIME_MIN]);
  if (p) {
    dateTime.tm_min = p->getIntValue(context);
  }
  p = properties.GetPtr(timeProperties[TIME_SEC]);
  if (p) {
    dateTime.tm_sec = p->getIntValue(context);
  }

  p = properties.GetPtr(dateProperties[DATE_YEAR]);
  if (p) {
    int new_year = p->getIntValue(context);
    if (checkYear(new_year, actionName)) {
      dateTime.tm_year = new_year - 1900;
    }
  }

  p = properties.GetPtr(dateProperties[DATE_MONTH]);
  if (p) {
    int new_mon = p->getIntValue(context);
    if (new_mon > 0) {
      dateTime.tm_mon = new_mon - 1;
    } else {
      smsc_log_warn(logger, "%s '%s' action : error month new value %d", DATETIME_ACTION,
                     actionName, new_mon);
    }
  }

  p = properties.GetPtr(dateProperties[DATE_MDAY]);
  if (p) {
    int new_day = p->getIntValue(context);
    if (new_day > 0) {
      dateTime.tm_mday = new_day;
    } else {
      smsc_log_warn(logger, "%s '%s' action : error day new value %d", DATETIME_ACTION,
                    actionName, new_day);
    }
  }
  setDateTime(actionName, DATETIME_FORMAT, dateTime, context);
  return true;
}

bool DateTimeProperty::add(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName, int sign) {
  struct tm dateTime;
  if (!getOldDateTime(context, actionName, dateTime)) {
    return false;
  }
  ActionProperty* p = properties.GetPtr(dateProperties[DATE_YEAR]);
  if (p) {
    dateTime.tm_year += p->getIntValue(context) * sign;
  }
  p = properties.GetPtr(dateProperties[DATE_MONTH]);
  if (p) {
    dateTime.tm_mon += p->getIntValue(context) * sign;
  }
  p = properties.GetPtr(dateProperties[DATE_MDAY]);
  if (p) {
    dateTime.tm_mday += p->getIntValue(context) * sign;
  }
  p = properties.GetPtr(timeProperties[TIME_HOUR]);
  if (p) {
    dateTime.tm_hour += p->getIntValue(context) * sign;
  }
  p = properties.GetPtr(timeProperties[TIME_MIN]);
  if (p) {
    dateTime.tm_min += p->getIntValue(context) * sign;
  }
  p = properties.GetPtr(timeProperties[TIME_SEC]);
  if (p) {
    dateTime.tm_sec += p->getIntValue(context) * sign;
  }
  setDateTime(actionName, DATETIME_FORMAT, dateTime, context);
  return true;
}

}
}
}
