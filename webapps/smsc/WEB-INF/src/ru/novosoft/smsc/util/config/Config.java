/*
* Created by igork
* Date: Mar 12, 2002
* Time: 2:56:46 PM
*/
package ru.novosoft.smsc.util.config;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.xml.Utils;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;


public class Config implements Cloneable
{
  File configFile = null;
  protected Map params = new HashMap();


  public static class ParamNotFoundException extends Exception
  {
    public ParamNotFoundException(String s)
    {
      super(s);
    }
  }


  public static class WrongParamTypeException extends Exception
  {
    public WrongParamTypeException(String s)
    {
      super(s);
    }
  }

  public Config(File configFile) throws IOException, SAXException, ParserConfigurationException
  {
    this.configFile = configFile;
    final FileReader reader = new FileReader(this.configFile);
    try {
      parseNode("", Utils.parse(reader).getDocumentElement());
    } finally {
      reader.close();
    }
  }

  public Config(Reader configReader) throws IOException, SAXException, ParserConfigurationException
  {
    parseNode("", Utils.parse(configReader).getDocumentElement());
  }

  public Object clone() throws CloneNotSupportedException
  {
    Config newConfig = (Config) super.clone();
    newConfig.configFile = new File(configFile.getAbsolutePath());
    newConfig.params = new HashMap(params);
    return newConfig;
  }

  public synchronized int getInt(String paramName) throws ParamNotFoundException, WrongParamTypeException
  {
    Object value = params.get(paramName);
    if (value == null)
      throw new ParamNotFoundException("Parameter \"" + paramName + "\" not found");
    if (value instanceof Integer)
      return ((Integer) value).intValue();
    else
      throw new WrongParamTypeException("Parameter \"" + paramName + "\" is not integer");
  }

  public synchronized String getString(String paramName) throws ParamNotFoundException, WrongParamTypeException
  {
    Object value = params.get(paramName);
    if (value == null)
      throw new ParamNotFoundException("Parameter \"" + paramName + "\" not found");
    if (value instanceof String)
      return (String) value;
    else
      throw new WrongParamTypeException("Parameter \"" + paramName + "\" is not string");
  }

  public synchronized boolean getBool(String paramName) throws ParamNotFoundException, WrongParamTypeException
  {
    Object value = params.get(paramName);
    if (value == null)
      throw new ParamNotFoundException("Parameter \"" + paramName + "\" not found");
    if (value instanceof Boolean)
      return ((Boolean) value).booleanValue();
    else
      throw new WrongParamTypeException("Parameter \"" + paramName + "\" is not boolean");
  }

  public synchronized Object getParameter(String paramName)
  {
    return params.get(paramName);
  }

  public synchronized Set getParameterNames()
  {
    return new HashSet(params.keySet());
  }

  public synchronized void setInt(String paramName, int value)
  {
    params.put(paramName, new Integer(value));
  }

  public synchronized void setString(String paramName, String value)
  {
    params.put(paramName, value);
  }

  public synchronized void setBool(String paramName, boolean value)
  {
    params.put(paramName, new Boolean(value));
  }

  public synchronized void removeParam(String paramName)
  {
    params.remove(paramName);
  }

  /**
   * ���� ����� ������ (������ ������)
   * @return section names that is immediate descedants of given section. Full names.
   */
  public synchronized Set getSectionChildSectionNames(String sectionName)
  {
    int dotpos = sectionName.length();
    Set result = new HashSet();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      String name = (String) i.next();
      if (name.length() > (dotpos + 1) && name.startsWith(sectionName) && name.charAt(dotpos) == '.' && name.lastIndexOf('.') > dotpos) {
        result.add(name.substring(0, name.indexOf('.', dotpos + 1)));
      }
    }
    return result;
  }

  /**
   * ���� ����� ������ (������ ������)
   * @return section names that is immediate descedants of given section. Full names.
   */
  public synchronized Set getSectionChildShortSectionNames(String sectionName)
  {
    int dotpos = sectionName.length();
    Set result = new HashSet();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      String name = (String) i.next();
      if (name.length() > (dotpos + 1) && name.startsWith(sectionName) && name.charAt(dotpos) == '.' && name.lastIndexOf('.') > dotpos) {
        result.add(name.substring(dotpos + 1, name.indexOf('.', dotpos + 1)));
      }
    }
    return result;
  }

  /**
   * ���� ����� ������ (������ ������)
   * @return section names that is immediate descedants of given section.
   */
  public synchronized Set getSectionChildParamsNames(String sectionName)
  {
    int dotpos = sectionName.length();
    Set result = new HashSet();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      String name = (String) i.next();
      if (name.length() > (dotpos + 1) && name.startsWith(sectionName) && name.lastIndexOf('.') == dotpos) {
        result.add(name);
      }
    }
    return result;
  }

  public synchronized void renameSection(String oldName, String newName)
  {
    List oldNames = new LinkedList();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      String oldParamName = (String) i.next();
      if (oldParamName.startsWith(oldName)) {
        oldNames.add(oldParamName);
      }
    }
    for (Iterator i = oldNames.iterator(); i.hasNext();) {
      String oldParamName = (String) i.next();
      Object value = params.remove(oldParamName);
      params.put(newName + oldParamName.substring(oldName.length()), value);
    }
  }

  protected void parseNode(final String prefix, final Element elem)
  {
    String fullName = prefix == null || prefix.equals("") ? elem.getAttribute("name") : prefix + "." + elem.getAttribute("name");

    NodeList list = elem.getChildNodes();
    for (int i = 0; i < list.getLength(); i++) {
      Node node = list.item(i);
      if (node.getNodeType() == Node.ELEMENT_NODE) {
        Element element = (Element) node;
        if (element.getNodeName().equals("section"))
          parseNode(fullName, element);
        else
          parseParamNode(fullName, element);
      }
    }
  }

  protected void parseParamNode(final String prefix, final Element elem)
  {
    String fullName = prefix == null || prefix.equals("") ? elem.getAttribute("name") : prefix + "." + elem.getAttribute("name");
    String type = elem.getAttribute("type");
    String value = Utils.getNodeText(elem);
    if (type.equalsIgnoreCase("int")) {
      params.put(fullName, new Integer(value));
    } else if (type.equalsIgnoreCase("bool")) {
      params.put(fullName, new Boolean(value));
    } else {
      params.put(fullName, value);
    }
  }

  public synchronized void removeSection(final String sectionName)
  {
    for (Iterator i = getSectionChildSectionNames(sectionName).iterator(); i.hasNext();)
      removeSection((String) i.next());
    for (Iterator i = getSectionChildParamsNames(sectionName).iterator(); i.hasNext();)
      removeParam((String) i.next());
  }

  /**
   * ���������� ������ � ��� ����, �� �������� �������� � ������������. ���� ������ ��� �������� �����
   * Config(Reader configReader) - �� ���� ���� ������� ���������� - ����� ������ NullPointerException <br>
   * � ���� ������� ����� �������� ��������� "ISO-8859-1"
   * @throws IOException
   * @throws WrongParamTypeException
   * @throws NullPointerException ���� ���������� ���� �������. ���� �� ������� ������ � �������
   * Config(Reader configReader), �� ������ ����� ��� ������ ������������ ����� save(File configFileToSave, String encoding)
   * @see #save(File configFileToSave)
   */
  public synchronized void save() throws IOException, WrongParamTypeException, NullPointerException
  {
    save("ISO-8859-1");
  }

  /**
   * ���������� ������ � ��������� ����.<br>
   * � ���� ������� ����� �������� ��������� "ISO-8859-1"
   * @throws IOException
   * @throws WrongParamTypeException
   */
  public synchronized void save(final File configFile) throws IOException, WrongParamTypeException
  {
    if (configFile == null)
      throw new NullPointerException("config file not specified");
    save(configFile, "ISO-8859-1");
  }

  /**
   * ���������� ������ � ��� ����, �� �������� �������� � ������������. ���� ������ ��� �������� �����
   * Config(Reader configReader) - �� ���� ���� ������� ���������� - ����� ������ NullPointerException
   * @param encoding - ���������, ������� ����� ������� � ����� �������. ������ �-���� xerces �������� ������
   * "ISO-8859-1"
   * @throws IOException
   * @throws WrongParamTypeException
   * @throws NullPointerException ���� ���������� ���� �������. ���� �� ������� ������ � �������
   * Config(Reader configReader), �� ������ ����� ��� ������ ������������ ����� save(File configFileToSave, String encoding)
   * @see #save(File configFileToSave, String encoding)
   */
  public synchronized void save(final String encoding) throws IOException, WrongParamTypeException, NullPointerException
  {
    if (configFile == null)
      throw new NullPointerException("config file not specified");
    save(configFile, encoding);
  }

  /**
   * ���������� ������ � ��������� ����.
   * @param encoding - ���������, ������� ����� ������� � ����� �������. ������ �-���� xerces �������� ������
   * "ISO-8859-1"
   * @param configFileToSave
   * @param encoding
   * @throws IOException
   * @throws WrongParamTypeException
   */
  public synchronized void save(final File configFileToSave, final String encoding) throws IOException, WrongParamTypeException
  {
    File configXmlNew = Functions.createNewFilenameForSave(configFileToSave);

    saveInternal(configXmlNew, encoding);

    Functions.renameNewSavedFileToOriginal(configXmlNew, configFileToSave);
  }

  private void saveInternal(File fileToSaveTo, final String encoding) throws IOException, WrongParamTypeException
  {
    // save new config to temp file
    SaveableConfigTree tree = new SaveableConfigTree(this);
    PrintWriter out = new PrintWriter(new FileWriter(fileToSaveTo));
    Functions.storeConfigHeader(out, "config", "configuration.dtd", encoding); // C++ code doesn't know about other codings // System.getProperty("file.encoding");
    tree.write(out, "  ");
    Functions.storeConfigFooter(out, "config");
    out.flush();
    out.close();
  }

  public void saveWithoutBackup() throws IOException, WrongParamTypeException
  {
    if (configFile == null)
      throw new NullPointerException("config file not specified");
    saveInternal(configFile, "ISO-8859-1");
  }

  public Collection getSectionChildShortParamsNames(String sectionName)
  {
    int dotpos = sectionName.length();
    Set result = new HashSet();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      String name = (String) i.next();
      if (name.length() > (dotpos + 1) && name.startsWith(sectionName) && name.lastIndexOf('.') == dotpos) {
        result.add(name.substring(dotpos + 1));
      }
    }
    return result;
  }

  public boolean containsSection(String sectionName)
  {
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      String paramName = (String) i.next();
      if (paramName.length() > sectionName.length() && paramName.charAt(sectionName.length()) == '.' && paramName.startsWith(sectionName))
        return true;
    }
    return false;
  }

  public boolean containsParameter(String parameterName)
  {
    return params.containsKey(parameterName);
  }

  public void copySectionFromConfig(final Config configToCopyFrom, final String sectionName)
  {
    final int sectionNameLength = sectionName.length();
    for (Iterator i = configToCopyFrom.getParameterNames().iterator(); i.hasNext();) {
      String paramName = (String) i.next();
      if (paramName.startsWith(sectionName) && paramName.charAt(sectionNameLength) == '.')
        params.put(paramName, configToCopyFrom.params.get(paramName));
    }
  }

  public static boolean isParamEquals(Config config1, Config config2, String fullParamName)
  {
    Object o1 = config1.params.get(fullParamName);
    Object o2 = config2.params.get(fullParamName);
    return (o1 == null && o2 == null)
            || (o1 != null && o2 != null && o1.equals(o2));
  }

  public boolean isParamEquals(Config anotherConfig, String fullParamName)
  {
    Object o1 = this.params.get(fullParamName);
    Object o2 = anotherConfig.params.get(fullParamName);
    return (o1 == null && o2 == null)
            || (o1 != null && o2 != null && o1.equals(o2));
  }

  public boolean isParamEquals(String fullParamName, Object paramValue)
  {
    Object o1 = this.params.get(fullParamName);
    return (o1 == null && paramValue == null)
            || (o1 != null && paramValue != null && o1.equals(paramValue));
  }

  public boolean isStringParamEquals(String fullParamName, String paramValue)
  {
    Object o1 = this.params.get(fullParamName);
    if (o1 instanceof String) {
      String s1 = (String) o1;
      return ((s1 == null || s1.length() == 0) && (paramValue == null || paramValue.length() == 0))
              || (s1 != null && s1.length() > 0 && paramValue != null && paramValue.length() > 0 && o1.equals(paramValue));
    } else
      return false;
  }

  public boolean isBooleanParamEquals(String fullParamName, boolean paramValue)
  {
    final Object o1 = this.params.get(fullParamName);
    if (o1 instanceof Boolean) {
      final Boolean b1 = (Boolean) o1;
      final boolean v1 = b1 == null ? false : b1.booleanValue();
      return v1 == paramValue;
    } else
      return false;
  }

  public boolean isIntParamEquals(String fullParamName, int paramValue)
  {
    final Object o1 = this.params.get(fullParamName);
    if (o1 instanceof Integer) {
      final Integer i1 = (Integer) o1;
      final int v1 = i1 == null ? 0 : i1.intValue();
      return v1 == paramValue;
    } else
      return false;
  }
}
