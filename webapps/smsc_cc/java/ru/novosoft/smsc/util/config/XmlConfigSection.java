package ru.novosoft.smsc.util.config;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 * 
 * Section of XmlConfig
 */

public class XmlConfigSection {

  protected Map<String, XmlConfigSection> sections = new HashMap<String, XmlConfigSection>(10);
  protected Map<String, XmlConfigParam> params = new HashMap<String, XmlConfigParam>(10);

  protected String name;

  /**
   * Creates new instance of XmlConfig section
   */
  public XmlConfigSection() {
    this(null);
  }

  /**
   * Creates new instance of XmlConfig section with specified name
   * @param name section name
   */
  public XmlConfigSection(String name) {
    this.name = name;
  }

  /**
   * Returns section name
   * @return section name
   */
  public String getName() {
    return name;
  }

  /**
   * Set new section name
   * @param name new section name
   */
  public void setName(String name) {
    this.name = name;
  }

  /**
   * Returns section content (includes subsections) as Properties object
   * @param prefix properties name prefix
   * @param sectionsDelimiter delimiter between subsections names
   * @return Section content as properties
   */
  public Properties toProperties(String prefix, String sectionsDelimiter) {
    final Properties props = new Properties();

    for (XmlConfigParam p : params())
      props.setProperty(prefix + p.getName(), p.getString());

    for (XmlConfigSection s : sections()) {
      Properties sprops = s.toProperties(prefix + s.getName() + sectionsDelimiter, sectionsDelimiter);

      for (Object pname : sprops.keySet())
        props.setProperty((String)pname, sprops.getProperty((String)pname));
    }

    return props;
  }

  /**
   * Returns section content (includes subsections) as Properties object
   * @param sectionsDelimiter delimiter between subsections names
   * @return Section content as properties
   */
  public Properties toProperties(String sectionsDelimiter) {
    return toProperties("", sectionsDelimiter);
  }


  /**
   * Add new section into this section
   * @param section new section
   */
  public void addSection(final XmlConfigSection section) {
    sections.put(section.name, section);
  }

  /**
   * Create new section with specified name inside this section
   * @param name new section name
   * @return new section
   */
  public XmlConfigSection addSection(final String name) {
    final XmlConfigSection section = new XmlConfigSection(name);
    sections.put(name, section);
    return section;
  }

  /**
   * Returns section with specified name of null if such section does not exists
   * @param name section name
   * @return section or null
   * @throws XmlConfigException if section with specified name does not exists
   */
  public XmlConfigSection getSection(final String name) throws XmlConfigException {
    XmlConfigSection s = sections.get(name);
    if (s == null)
      throw new XmlConfigException("Section " + name + " not found in " + this.name + " section.");
    return s;
  }

    /**
   * Returns section with specified name, create it if it doesn't exist
   * @param name section name
   * @return section
   * @throws XmlConfigException if section with specified name does not exists
   */
  public XmlConfigSection getOrCreateSection(String name) throws XmlConfigException {
    return containsSection(name) ? getSection(name) : addSection(name);
  }

  /**
   * Check section contains subsection with specified name
   * @param name subsection name
   * @return true, if current section contains subsection with specified name, otherwise returns false
   */
  public boolean containsSection(String name) {
    return sections.containsKey(name);
  }

  /**
   * Removes section with specified name
   * @param name section name
   * @return removed section
   */
  public XmlConfigSection removeSection(final String name) {
    return sections.remove(name);
  }

  /**
   * Remove section
   * @param section section to remove
   * @return result of operation
   */
  public boolean removeSection(final XmlConfigSection section) {
    return sections.remove(section.name) != null;
  }

  /**
   * Remove all sections
   */
  public void clear() {
    clearSections();
    clearParams();
  }

  /**
   * Remove all params from section
   */
  public void clearParams() {
    params.clear();
  }

  /**
   * Remove all subsections
   */
  public void clearSections() {
    sections.clear();
  }

  /**
   * Returns sections iterator
   * @return sections iterator
   */
  public Collection<XmlConfigSection> sections() {
    return new ArrayList<XmlConfigSection>(sections.values());
  }

  /**
   * Returns count of all sections
   * @return sectins count
   */
  public int getSectionsCount() {
    return sections.values().size();
  }

  /**
   * Add parameter into section
   * @param param new parameter
   */
  public void addParam(final XmlConfigParam param) {
    if (param != null)
      params.put(param.getName(), param);
  }

  /**
   * Create new parameter with specified name inside section
   * @param name parameter name
   * @return new parameter
   */
  public XmlConfigParam addParam(final String name) {
    final XmlConfigParam param = new XmlConfigParam(name);
    params.put(name, param);
    return param;
  }

  /**
   * Returns parameter with specified name or null if parameter does not exist
   * @param name parameter name
   * @return parameter or null
   */
  public XmlConfigParam getParam(final String name) {
    return params.get(name);
  }

  /**
   * Returns parameter with specified name. If such parametrer does not exists, 
   * creates new one.
   * @param name parameter name
   * @return parameter
   */
  public XmlConfigParam getOrCreateParam(final String name) {
    XmlConfigParam param = getParam(name);
    if (param == null) {
      param = new XmlConfigParam(name);
      params.put(name, param);
    }
    return param;
  }

  /**
   * Removes parameter with specified name
   * @param name parameter name
   * @return removed parameter
   */
  public XmlConfigParam removeParam(final String name) {
    return params.remove(name);
  }

  /**
   * Remove specified parameter from section
   * @param param parameter
   * @return result of operation
   */
  public boolean removeParam(final XmlConfigParam param) {
    return params.remove(param.getName()) != null;
  }

  /**
   * Check section contains param with specified name
   * @param param param name
   * @return true, if section contains param with specified name, otherwise returns false
   */
  public boolean containsParam(String param) {
    return params.containsKey(param);
  }

  /**
   * Returns count of parameters
   * @return parameters count
   */
  public int getParamsCount() {
    return params.values().size();
  }

  /**
   * Returns parameters iterator
   * @return parameters 
   */
  public Collection<XmlConfigParam> params() {
    return new ArrayList<XmlConfigParam>(params.values());
  }

  private XmlConfigParam _getParam(String paramName) throws XmlConfigException {
    final XmlConfigParam param = getParam(paramName);
    if (param == null)
      throw new XmlConfigException("Param " + paramName + " not found in " + name + " section.");
    return param;
  }

  /**
   * Returns parameter value as string
   * @param paramName param name
   * @return string value of param
   * @throws XmlConfigException if param not found
   */
  public String getString(String paramName) throws XmlConfigException {
    return _getParam(paramName).getString();
  }

  /**
   * Returns parameter value as string or default value, if parameter does not exists
   * @param paramName param name
   * @param defaultValue default value
   * @return param value or default value
   */
  public String getString(String paramName, String defaultValue) {
    final XmlConfigParam param = getParam(paramName);
    return (param==null) ? defaultValue : param.getString();
  }

  /**
   * Returns param value as int
   * @param paramName param name
   * @return param value
   * @throws XmlConfigException if param not found or param value is not int
   */
  public int getInt(String paramName) throws XmlConfigException {
    return _getParam(paramName).getInt();
  }

  /**
   * Returns param value as int or default value if param does not exists
   * @param paramName param name
   * @param defaultValue default value
   * @return param or default value
   * @throws XmlConfigException if param value is not int
   */
  public int getInt(String paramName, int defaultValue) throws XmlConfigException {
    final XmlConfigParam param = getParam(paramName);
    return (param==null) ? defaultValue : param.getInt();
  }

  /**
   * Returns param value as long
   * @param paramName param name
   * @return param value
   * @throws XmlConfigException if param not found or param value is not long
   */
  public long getLong(String paramName) throws XmlConfigException {
    return _getParam(paramName).getLong();
  }

  /**
   * Returns param value as long or default value if param does not exists
   * @param paramName param name
   * @param defaultValue default value
   * @return param or default value
   * @throws XmlConfigException if param value is not long
   */
  public long getLong(String paramName, long defaultValue) throws XmlConfigException {
    final XmlConfigParam param = getParam(paramName);
    return (param==null) ? defaultValue : param.getLong();
  }

  /**
   * Returns param value(true,false) as bool or default value if param does not exists
   * @param paramName param name
   * @return param or default value
   * @throws XmlConfigException if param value is not bool
   */
  public boolean getBool(String paramName) throws XmlConfigException {
    return _getParam(paramName).getBool();
  }

  /**
   * Returns param value(true,false) as bool or default value if param does not exists
   * @param paramName param name
   * @param defaultValue default value
   * @return param or default value
   * @throws XmlConfigException if param value is not bool
   */
  public boolean getBool(String paramName, boolean defaultValue) throws XmlConfigException {
    final XmlConfigParam param = getParam(paramName);
    return (param==null) ? defaultValue : param.getBool();
  }

  /**
   * Return param value as int array.
   * @param paramName param name
   * @param delimiter values delimiter
   * @return int array
   * @throws XmlConfigException if param does not exists or has incorrect format
   */
  public int[] getIntArray(String paramName, String delimiter) throws XmlConfigException {
    return _getParam(paramName).getIntArray(delimiter);
  }

  /**
   * Return param value as long array.
   * @param paramName param name
   * @param delimiter values delimiter
   * @return long array
   * @throws XmlConfigException if param does not exists or has incorrect format
   */
  public long[] getLongArray(String paramName, String delimiter) throws XmlConfigException {
    return _getParam(paramName).getLongArray(delimiter);
  }

  /**
   * Return param value as string array.
   * @param paramName param name
   * @param delimiter values delimiter
   * @return string array
   * @throws XmlConfigException if param does not exists or has incorrect format
   */
  public String[] getStringArray(String paramName, String delimiter) throws XmlConfigException {
    return _getParam(paramName).getStringArray(delimiter);
  }

  /**
   * Return param value as string list.
   * @param paramName param name
   * @param delimiter values delimiter
   * @return string list
   * @throws XmlConfigException if param does not exists or has incorrect format
   */
  public List getStringList(String paramName, String delimiter) throws XmlConfigException {
    return _getParam(paramName).getStringList(delimiter);
  }

  /**
   * Return param value as date
   * @param paramName param name
   * @param dateFormat date format
   * @return param value as date
   * @throws XmlConfigException if param does not exist
   * @throws ParseException if param has invalid format
   */
  public Date getDate(String paramName, String dateFormat) throws XmlConfigException, ParseException {
    SimpleDateFormat df = new SimpleDateFormat(dateFormat);
    return df.parse(_getParam(paramName).getString());
  }

  /**
   * Return param value as date or default value, if param does not exist
   * @param paramName param name
   * @param dateFormat date format
   * @param defaultValue default value
   * @return param value as date or default value, if param does not exist
   * @throws ParseException
   */
  public Date getDate(String paramName, String dateFormat, Date defaultValue) throws ParseException {
    final XmlConfigParam param = getParam(paramName);
    if (param == null)
      return defaultValue;
    SimpleDateFormat df = new SimpleDateFormat(dateFormat);
    return df.parse(param.getString());
  }


  // SETTERS ===========================================================================================================

  /**
   * Set string value to param. If param does not exists, create it
   * @param name param name
   * @param string value
   * @return this
   */
  public XmlConfigSection setString(String name, final String string) {
    getOrCreateParam(name).setString(string);
    return this;
  }

  /**
   * Set int value to param. If param does not exists, create it
   * @param name param name
   * @param value value
   * @return this
   */
  public XmlConfigSection setInt(String name, int value) {
    getOrCreateParam(name).setInt(value);
    return this;
  }

  /**
   * Set long value to param. If param does not exists, create it
   * @param name param name
   * @param value value
   * @return this
   */
  public XmlConfigSection setLong(String name, long value) {
    getOrCreateParam(name).setLong(value);
    return this;
  }

  /**
   * Set date value to param. If param does not exists, create it
   * @param name param name
   * @param date value
   * @param dateFormat format of date
   * @return this
   */
  public XmlConfigSection setDate(String name, Date date, String dateFormat) {
    getOrCreateParam(name).setDate(date, dateFormat);
    return this;
  }

  /**
   * Set bool value to param. If param does not exists, create it
   * @param name param name
   * @param value value
   * @return this
   */
  public XmlConfigSection setBool(String name, boolean value) {
    getOrCreateParam(name).setBool(value);
    return this;
  }

  /**
   * Set string list value to param. If param does not exists, create it
   * @param name param name
   * @param stringList strings collection
   * @param delimiter delimiter between strings in param value
   * @return this
   * @throws XmlConfigException if can't transform collection into string
   */
  public XmlConfigSection setStringList(String name, Collection stringList, String delimiter) throws XmlConfigException {
    getOrCreateParam(name).setStringList(stringList, delimiter);
    return this;
  }

  /**
   * Set string array value to param. If param does not exists, create it
   * @param name param name
   * @param stringList array of strings
   * @param delimiter delimiter delimiter between strings in param value
   * @return this
   */
  public XmlConfigSection setStringArray(String name, String[] stringList, String delimiter) {
    getOrCreateParam(name).setStringArray(stringList, delimiter);
    return this;
  }

  /**
   * Set long array value to param. If param does not exists, create it
   * @param name param name
   * @param values array of longs
   * @param delimiter delimiter delimiter between longs in param value
   * @return this
   */
  public XmlConfigSection setLongArray(String name, long[] values, String delimiter) {
    getOrCreateParam(name).setLongArray(values, delimiter);
    return this;
  }

  /**
   * Set int array value to param. If param does not exists, create it
   * @param name param name
   * @param values array of int's
   * @param delimiter delimiter delimiter between ints in param value
   * @return this
   */
  public XmlConfigSection setIntArray(String name, int[] values, String delimiter) {
    getOrCreateParam(name).setIntArray(values, delimiter);
    return this;
  }

  public Object clone() throws CloneNotSupportedException {
    XmlConfigSection section = (XmlConfigSection)super.clone();
    section.name = name;
    for(Map.Entry<String, XmlConfigSection> ss : sections.entrySet()) {
      section.sections.put(ss.getKey(), (XmlConfigSection)ss.getValue().clone());
    }
    for(Map.Entry<String, XmlConfigParam> pp : params.entrySet()) {
      section.params.put(pp.getKey(), (XmlConfigParam)pp.getValue().clone());
    }
    return section;
  }
}
