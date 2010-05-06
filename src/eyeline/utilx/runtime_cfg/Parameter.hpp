#ifndef __EYELINE_UTILX_RUNTIMECFG_PARAMETER_HPP__
# define __EYELINE_UTILX_RUNTIMECFG_PARAMETER_HPP__

# include <string>
# include <map>

namespace eyeline {
namespace utilx {
namespace runtime_cfg {

class CompositeParameter;

class Parameter {
public:
  Parameter(const std::string& name, const std::string& value);
  Parameter(const std::string& name, int value);

  virtual ~Parameter() {}

  const std::string& getName() const;

  const std::string& getValue() const;
  int getIntValue() const;

  void setValue(const std::string& value);
  void setIntValue(int value);

  const std::string& getFullName() const;

  bool isSetValue() const;

  virtual bool isComposite() const { return false; }

  typedef std::multimap<std::string, Parameter*>::iterator iterator_type_t;
  typedef std::multimap<std::string, Parameter*> container_type_t;

  std::string printParamaterValue() const;

protected:
  explicit Parameter(const std::string& name);

  void setParameterPrefix(const std::string& parameterPrefix);
  friend class CompositeParameter; // to grant access to setParameterPrefix() from CompositeParameter::addParameter methods

private:
  std::string _name, _fullName, _value;
  bool _isSetValue, _isSetIntValue;
  int _intValue;
  Parameter(const Parameter& rhs);
  Parameter& operator=(const Parameter& rhs);
};

}}}

#endif
