#ifndef __SCAG_RULE_ENGINE_ACTION_DATETIME2__
#define __SCAG_RULE_ENGINE_ACTION_DATETIME2__

#include "scag/re/actions/Action2.h"
#include "core/buffers/Hash.hpp"
#include "scag/util/properties/Properties2.h"
#include <vector>

namespace scag2 {
namespace re {
namespace actions {

using std::vector;
using std::string;
using smsc::core::buffers::Hash;
using util::properties::Property;
// using scag::util::properties::Property;

struct ActionProperty {
  void setStrValue(const string& _value, ActionContext &context) const {
    if (type == ftUnknown) {
      return;
    }
    Property *p = context.getProperty(value);
    if (p) {
      p->setStr(_value);
    }
  }
  string getStrValue(ActionContext &context) const {
    if (type == ftUnknown) {
      return value;
    } 
    Property *p = context.getProperty(value);
    if (!p) {
      return "";
    }
    return p->getStr();
  }
  int getIntValue(ActionContext &context) const {
    if (type == ftUnknown) {
      return intValue;
    }
    Property *p = context.getProperty(value);
    if (!p) {
      return 0;
    }
    return int(p->getInt());
  }
  void setIntValue(int _value, ActionContext &context) const {
    if (type == ftUnknown) {
      return;
    } 
    Property *p = context.getProperty(value);
    if (p) {
      p->setInt(_value);
    }
  }
  ActionProperty():type(ftUnknown), intValue(0) { 
    logger = Logger::getInstance("re.actions"); 
  };
  ActionProperty(FieldType _type, const string _value):type(_type), value(_value) {
    logger = Logger::getInstance("re.actions");
  };
  bool setValue(const string& _value, bool isInt = true) {
    smsc_log_debug(logger, "set value = '%s' is int %d ft %d", _value.c_str(), (int)isInt, type);
    if (type != ftUnknown || !isInt) {
      value = _value;
      return true;
    }
    intValue = atoi(_value.c_str());
    smsc_log_debug(logger, "set int value = %d", intValue);
    if (_value[0] != '0' && intValue == 0) {
      return false;
    }
    return true;
  }
  void setType(FieldType ft) {
    type = ft;
  }
  //virtual bool change(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName) {
     //return true;
 // };
  virtual ~ActionProperty() {};
protected:
  bool checkYear(int year);
protected:
  FieldType type;
  string value;
  int intValue;
  Logger *logger;
};

class DateTimeModifier {
public:
  DateTimeModifier(FieldType _type, const string _value):property(_type, _value) {
    logger = Logger::getInstance("re.actions");
  };
  virtual bool change(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName) = 0;
  virtual bool add(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName, int sign) = 0;
  virtual ~DateTimeModifier() {};
protected:
  virtual bool getOldDateTime(ActionContext &context, const char* actionName, struct tm& dateTime) = 0;
  virtual bool setDateTime(const char* actionName, const char* datetimeFormat, struct tm& dateTime, ActionContext& context);
  bool checkYear(int year, const char* actionName);
protected:
  ActionProperty property;
  Logger* logger;
};

class DateProperty: public DateTimeModifier {
public:
  DateProperty(FieldType _type, const string _value):DateTimeModifier(_type, _value) {};
  bool change(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName);
  bool add(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName, int sign);
protected:
  virtual bool getOldDateTime(ActionContext &context, const char* actionName, struct tm& dateTime) ;
};

class TimeProperty: public DateTimeModifier {
public:
  TimeProperty(FieldType _type, const string _value):DateTimeModifier(_type, _value) {};
  bool change(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName);
  bool add(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName, int sign);
protected:
  virtual bool getOldDateTime(ActionContext &context, const char* actionName, struct tm& dateTime);
};

class DateTimeProperty: public DateTimeModifier {
public:
  DateTimeProperty(FieldType _type, const string _value):DateTimeModifier(_type, _value) {};
  bool change(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName);
  bool add(ActionContext &context, Hash<ActionProperty>& properties, const char* actionName, int sign);
protected:
  virtual bool getOldDateTime(ActionContext &context, const char* actionName, struct tm& dateTime);
};


class DateTimeAction: public Action {
public:
  DateTimeAction():isDst(false), dateTimeProperty(0) {};
  virtual void init(const SectionParams &params, PropertyObject propertyObject) = 0;
  virtual bool run(ActionContext &context) = 0;
  virtual ~DateTimeAction() { if (dateTimeProperty) delete dateTimeProperty; };
protected:
  virtual const char* getActionName() const = 0;
  bool addProperty(const SectionParams &params, PropertyObject &propertyObject,
                   const char* propertyName, bool isInt, bool isRequired, bool readOnly);
  bool getDst(const SectionParams &params, PropertyObject &propertyObject);
  void intiDateModifierAction(const SectionParams &params, PropertyObject& propertyObject);
  virtual IParserHandler * StartXMLSubSection(const std::string &name, const SectionParams &params,
                                               const ActionFactory &factory);
  virtual bool FinishXMLSubSection(const std::string &name);

protected:
  Hash<ActionProperty> properties;
  bool isDst;
  DateTimeModifier* dateTimeProperty;
};

class CurrentDateTimeAction: public DateTimeAction {
public:
  CurrentDateTimeAction() {};
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual bool run(ActionContext &context);
  virtual const char* getActionName() const;
  virtual ~CurrentDateTimeAction() {};
private:
  CurrentDateTimeAction(const CurrentDateTimeAction&);
  bool checkTimeZone(int timeZone);

};

class ChangeDateTimeAction: public DateTimeAction {
public:
  ChangeDateTimeAction() {};
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual bool run(ActionContext &context);
  virtual const char* getActionName() const;
  virtual ~ChangeDateTimeAction() {};
private:
  ChangeDateTimeAction(const ChangeDateTimeAction&);
};

class AddDateTimeAction: public DateTimeAction {
public:
  AddDateTimeAction():sign(1) {};
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual bool run(ActionContext &context);
  virtual const char* getActionName() const;
  virtual ~AddDateTimeAction() {};
private:
  AddDateTimeAction(const ChangeDateTimeAction&);
private:
  int sign;
};

class DecDateTimeAction: public DateTimeAction {
public:
  DecDateTimeAction():sign(-1) {};
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual bool run(ActionContext &context);
  virtual const char* getActionName() const;
  virtual ~DecDateTimeAction() {};
private:
  DecDateTimeAction(const ChangeDateTimeAction&);
private:
  int sign;
};

}
}
}

#endif
