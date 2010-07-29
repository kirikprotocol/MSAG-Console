/*
* Created by igork
* Date: Mar 12, 2002
* Time: 2:56:46 PM
*/
package ru.sibinco.lib.backend.util.config;

import org.w3c.dom.*;
import org.xml.sax.SAXException;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.xml.Utils;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;


public class Config implements Cloneable
{
  private File configFile = null;
  protected Map params = new HashMap();


  public static class ParamNotFoundException extends Exception
  {
    public ParamNotFoundException(final String s)
    {
      super(s);
    }
  }


  public static class WrongParamTypeException extends Exception
  {
    public WrongParamTypeException(final String s)
    {
      super(s);
    }
  }


  public Config(final File configFile) throws IOException, SAXException, ParserConfigurationException
  {
    this.configFile = configFile;
    final FileReader reader;
    try {
      reader = new FileReader(this.configFile);
      parseNode("", Utils.parse(reader).getDocumentElement());
      reader.close();
    } catch (FileNotFoundException e) {
      System.out.println("Failed to open config file \"" + this.configFile.getAbsolutePath() + '"');
      throw e;
    }
  }

  public Config(final Reader configReader) throws IOException, SAXException, ParserConfigurationException
  {
    parseNode("", Utils.parse(configReader).getDocumentElement());
  }

  public Object clone() throws CloneNotSupportedException
  {
    final Config newConfig = (Config) super.clone();
    newConfig.configFile = new File(configFile.getAbsolutePath());
    newConfig.params = new HashMap(params);
    return newConfig;
  }

  public synchronized long getInt(final String paramName) throws ParamNotFoundException, WrongParamTypeException
  {
    final Object value = params.get(paramName);
    if (null == value)
      throw new ParamNotFoundException("Parameter \"" + paramName + "\" not found");
    if (value instanceof Integer)
      return ((Integer) value).intValue();
    else if (value instanceof Long)
      return ((Long) value).longValue();
    throw new WrongParamTypeException("Parameter \"" + paramName + "\" is not integer");
  }

  public synchronized String getString(final String paramName) throws ParamNotFoundException, WrongParamTypeException
  {
    final Object value = params.get(paramName);
    if (null == value)
      throw new ParamNotFoundException("Parameter \"" + paramName + "\" not found");
    if (value instanceof String)
      return (String) value;
    else
      throw new WrongParamTypeException("Parameter \"" + paramName + "\" is not string");
  }

  public synchronized boolean getBool(final String paramName) throws ParamNotFoundException, WrongParamTypeException
  {
    final Object value = params.get(paramName);
    if (null == value)
      throw new ParamNotFoundException("Parameter \"" + paramName + "\" not found");
    if (value instanceof Boolean)
      return ((Boolean) value).booleanValue();
    else
      throw new WrongParamTypeException("Parameter \"" + paramName + "\" is not boolean");
  }

  public synchronized Object getParameter(final String paramName)
  {
    return params.get(paramName);
  }

  public synchronized Set getParameterNames()
  {
    return new HashSet(params.keySet());
  }

  public synchronized void setInt(final String paramName, final long value)
  {
    params.put(paramName, new Long(value));
  }

  public synchronized void setString(final String paramName, final String value)
  {
    params.put(paramName, value);
  }

  public synchronized void setBool(final String paramName, final boolean value)
  {
    params.put(paramName, new Boolean(value));
  }

  public synchronized void removeParam(final String paramName)
  {
    params.remove(paramName);
  }

  /**
   * Ищет имена секций (только секций)
   *
   * @return section names that is immediate descedants of given section. Full names.
   */
  public synchronized Set getSectionChildSectionNames(final String sectionName)
  {
    final int dotpos = sectionName.length();
    final Set result = new HashSet();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      final String name = (String) i.next();
      if (name.length() > (dotpos + 1) && name.startsWith(sectionName) && '.' == name.charAt(dotpos) && name.lastIndexOf('.') > dotpos) {
        result.add(name.substring(0, name.indexOf('.', dotpos + 1)));
      }
    }
    return result;
  }

  /**
   * Ищет имена секций (только секций)
   *
   * @return section names that is immediate descedants of given section. Full names.
   */
  public synchronized Set getSectionChildShortSectionNames(final String sectionName)
  {
    final int dotpos = sectionName.length();
    final Set result = new HashSet();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      final String name = (String) i.next();
      if (name.length() > (dotpos + 1) && name.startsWith(sectionName) && '.' == name.charAt(dotpos) && name.lastIndexOf('.') > dotpos) {
        result.add(name.substring(dotpos + 1, name.indexOf('.', dotpos + 1)));
      }
    }
    return result;
  }

  /**
   * Ищет имена секций (только секций)
   *
   * @return section names that is immediate descedants of given section.
   */
  public synchronized Set getSectionChildParamsNames(final String sectionName)
  {
    final int dotpos = sectionName.length();
    final Set result = new HashSet();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      final String name = (String) i.next();
      if (name.length() > (dotpos + 1) && name.startsWith(sectionName) && name.lastIndexOf('.') == dotpos) {
        result.add(name);
      }
    }
    return result;
  }

  public synchronized void renameSection(final String oldName, final String newName)
  {
    final List oldNames = new LinkedList();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      final String oldParamName = (String) i.next();
      if (oldParamName.startsWith(oldName)) {
        oldNames.add(oldParamName);
      }
    }
    for (Iterator i = oldNames.iterator(); i.hasNext();) {
      final String oldParamName = (String) i.next();
      final Object value = params.remove(oldParamName);
      params.put(newName + oldParamName.substring(oldName.length()), value);
    }
  }

  protected void parseNode(final String prefix, final Element elem)
  {
    final String fullName = null == prefix || "".equals(prefix) ? elem.getAttribute("name") : prefix + "." + elem.getAttribute("name");

    final NodeList list = elem.getChildNodes();
    for (int i = 0; i < list.getLength(); i++) {
      final Node node = list.item(i);
      if (Node.ELEMENT_NODE == node.getNodeType()) {
        final Element element = (Element) node;
        if ("section".equals(element.getNodeName()))
          parseNode(fullName, element);
        else
          parseParamNode(fullName, element);
      }
    }
  }

  protected void parseParamNode(final String prefix, final Element elem)
  {
    final String fullName = null == prefix || "".equals(prefix) ? elem.getAttribute("name") : prefix + "." + elem.getAttribute("name");
    final String type = elem.getAttribute("type");
    final String value = Utils.getNodeText(elem);
    if ("int".equalsIgnoreCase(type)) {
      params.put(fullName, new Integer(value));
    } else if ("bool".equalsIgnoreCase(type)) {
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
   * Записывает конфиг в тот файл, из которого прочитал в конструкторе. Если конфиг был построен через Config(Reader configReader) - то есть файл конфига
   * неизвестен - будет брошен NullPointerException <br> В файл конфига будет записана та кодировка, под которой запущен сервлет контейнер.
   *
   * @throws IOException
   * @throws WrongParamTypeException
   * @throws NullPointerException    если неизвестен файл конфига. Если вы создаёте конфиг с помощью Config(Reader configReader), то будьте добры для записи
   *                                 использовать метод save(File configFileToSave, String encoding)
   * @see #save(File configFileToSave)
   */
  public synchronized void save() throws IOException, WrongParamTypeException, NullPointerException
  {
    save(Functions.getLocaleEncoding());
  }

  /**
   * Записывает конфиг в указанный файл.<br> В файл конфига будет записана та кодировка, под которой запущен сервлет контейнер.
   *
   * @throws IOException
   * @throws WrongParamTypeException
   */
  public synchronized void save(final File configFile) throws IOException, WrongParamTypeException
  {
    if (null == configFile)
      throw new NullPointerException("config file not specified");
    save(configFile, Functions.getLocaleEncoding());
  }

  /**
   * Записывает конфиг в тот файл, из которого прочитал в конструкторе. Если конфиг был построен через Config(Reader configReader) - то есть файл конфига
   * неизвестен - будет брошен NullPointerException
   *
   * @param encoding - кодировка, которая будет указана в файле конфига.
   * @throws IOException
   * @throws WrongParamTypeException
   * @throws NullPointerException    если неизвестен файл конфига. Если вы создаёте конфиг с помощью Config(Reader configReader), то будьте добры для записи
   *                                 использовать метод save(File configFileToSave, String encoding)
   * @see #save(File configFileToSave, String encoding)
   */
  private synchronized void save(final String encoding) throws IOException, WrongParamTypeException, NullPointerException
  {
    if (null == configFile)
      throw new NullPointerException("config file not specified");
    save(configFile, encoding);
  }

  /**
   * Записывает конфиг в указанный файл.
   *
   * @param encoding         - кодировка, которая будет указана в файле конфига.
   * @param configFileToSave
   * @param encoding
   * @throws IOException
   * @throws WrongParamTypeException
   */
  private synchronized void save(final File configFileToSave, final String encoding) throws IOException, WrongParamTypeException
  {
    final File configXmlNew = Functions.createNewFilenameForSave(configFileToSave);

    saveInternal(configXmlNew, encoding);

    Functions.renameNewSavedFileToOriginal(configXmlNew, configFileToSave);
  }

  private void saveInternal(final File fileToSaveTo, final String encoding) throws IOException, WrongParamTypeException
  {
    // save new config to temp file
    final SaveableConfigTree tree = new SaveableConfigTree(this);
    final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(fileToSaveTo), Functions.getLocaleEncoding()));
    Functions.storeConfigHeader(out, "config", "configuration.dtd", encoding); // C++ code doesn't know about other codings // System.getProperty("file.encoding");
    tree.write(out, "  ");
    Functions.storeConfigFooter(out, "config");
    out.flush();
    out.close();
  }

  public void saveWithoutBackup() throws IOException, WrongParamTypeException
  {
    if (null == configFile)
      throw new NullPointerException("config file not specified");
    saveInternal(configFile, Functions.getLocaleEncoding());
  }

  public void saveWithoutBackup(String encoding) throws IOException, WrongParamTypeException
  {
    if (null == configFile)
      throw new NullPointerException("config file not specified");
    saveInternal(configFile, encoding);
  }

  public synchronized Collection getSectionChildShortParamsNames(final String sectionName)
  {
    final int dotpos = sectionName.length();
    final Set result = new HashSet();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      final String name = (String) i.next();
      if (name.length() > (dotpos + 1) && name.startsWith(sectionName) && name.lastIndexOf('.') == dotpos) {
        result.add(name.substring(dotpos + 1));
      }
    }
    return result;
  }

  public synchronized boolean containsSection(final String sectionName)
  {
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      final String paramName = (String) i.next();
      if (paramName.length() > sectionName.length() && '.' == paramName.charAt(sectionName.length()) && paramName.startsWith(sectionName))
        return true;
    }
    return false;
  }

  public synchronized boolean containsParameter(final String parameterName)
  {
    return params.containsKey(parameterName);
  }

  public synchronized void copySectionFromConfig(final Config configToCopyFrom, final String sectionName)
  {
    final int sectionNameLength = sectionName.length();
    for (Iterator i = configToCopyFrom.getParameterNames().iterator(); i.hasNext();) {
      final String paramName = (String) i.next();
      if (paramName.startsWith(sectionName) && '.' == paramName.charAt(sectionNameLength))
        params.put(paramName, configToCopyFrom.params.get(paramName));
    }
  }

  public static boolean isParamEquals(final Config config1, final Config config2, final String fullParamName)
  {
    final Object o1 = config1.params.get(fullParamName);
    final Object o2 = config2.params.get(fullParamName);
    return (null == o1 && null == o2)
           || (null != o1 && null != o2 && o1.equals(o2));
  }

  public synchronized boolean isParamEquals(final Config anotherConfig, final String fullParamName)
  {
    final Object o1 = this.params.get(fullParamName);
    final Object o2 = anotherConfig.params.get(fullParamName);
    return (null == o1 && null == o2)
           || (null != o1 && null != o2 && o1.equals(o2));
  }

  public synchronized boolean isParamEquals(final String fullParamName, final Object paramValue)
  {
    final Object o1 = this.params.get(fullParamName);
    return (null == o1 && null == paramValue)
           || (null != o1 && null != paramValue && o1.equals(paramValue));
  }

  public synchronized boolean isStringParamEquals(final String fullParamName, final String paramValue)
  {
    final Object o1 = this.params.get(fullParamName);
    if (null == o1 || o1 instanceof String) {
      final String s1 = (String) o1;
      return ((null == s1 || 0 == s1.length()) && (null == paramValue || 0 == paramValue.length()))
             || (null != s1 && 0 < s1.length() && null != paramValue && 0 < paramValue.length() && s1.equals(paramValue));
    } else
      return false;
  }

  public synchronized boolean isBooleanParamEquals(final String fullParamName, final boolean paramValue)
  {
    final Object o1 = this.params.get(fullParamName);
    if (null == o1 || o1 instanceof Boolean) {
      final Boolean b1 = (Boolean) o1;
      final boolean v1 = null == b1 ? false : b1.booleanValue();
      return v1 == paramValue;
    } else
      return false;
  }

  public synchronized boolean isIntParamEquals(final String fullParamName, final int paramValue)
  {
    final Object o1 = this.params.get(fullParamName);
    if (null == o1 || o1 instanceof Integer) {
      final Integer i1 = (Integer) o1;
      final int v1 = null == i1 ? 0 : i1.intValue();
      return v1 == paramValue;
    } else
      return false;
  }

  public synchronized void removeParamsFromSection(final String sectionName)
  {
    final int dotpos = sectionName.length();
    for (Iterator i = new ArrayList(params.keySet()).iterator(); i.hasNext();) {
      final String name = (String) i.next();
      if (name.length() > (dotpos + 1) && name.startsWith(sectionName) && name.lastIndexOf('.') == dotpos) {
        params.remove(name);
      }
    }
  }

  public synchronized void copySectionParamsFromConfig(final Config configToCopyFrom, final String sectionName)
  {
    final int sectionNameLength = sectionName.length();
    for (Iterator i = configToCopyFrom.getParameterNames().iterator(); i.hasNext();) {
      final String paramName = (String) i.next();
      if (paramName.length() > (sectionNameLength + 1) && paramName.startsWith(sectionName) && paramName.lastIndexOf('.') == sectionNameLength)
        params.put(paramName, configToCopyFrom.params.get(paramName));
    }
  }

  public void copyParams(final Config configToCopyFrom) {
    params.clear();
    params.putAll(configToCopyFrom.params);
  }
}
