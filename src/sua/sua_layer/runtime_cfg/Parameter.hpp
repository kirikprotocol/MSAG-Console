#ifndef PARAMETER_HPP_HEADER_INCLUDED_B8C7F6D0
# define PARAMETER_HPP_HEADER_INCLUDED_B8C7F6D0

# include <string>
# include <map>

namespace runtime_cfg {

class Parameter {
private:
  std::string _name, _fullName, _value;

  Parameter(const Parameter& rhs);
  Parameter& operator=(const Parameter& rhs);
public:
  void setParameterPrefix(const std::string& parameterPrefix);
  //##ModelId=4737DFCC0081
  Parameter(const std::string& name, const std::string& value);
  Parameter(const std::string& name, unsigned int value);

  virtual ~Parameter() {}

  //##ModelId=4737DFB50227
  const std::string& getName() const;

  //##ModelId=4737DFA60289
  const std::string& getValue() const;

  const std::string& getFullName() const;

  virtual bool isComposite() const { return false; }

  typedef std::multimap<std::string, Parameter*>::const_iterator iterator_trait_t;
};

}

#endif /* PARAMETER_HPP_HEADER_INCLUDED_B8C7F6D0 */
