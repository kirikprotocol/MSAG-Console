/*
 * Created by igork
 * Date: Mar 12, 2002
 * Time: 2:53:22 PM
 */
package ru.novosoft.smsc.util.config;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;


public class ConfigManager
{
  static protected boolean isInitialized = false;
  static protected String configurationFileName = "config.xml";
  static protected String configurationFileDir = "";
  static protected ConfigManager monitor = null;

  protected Config config = null;


  static public class IsNotInitializedException extends Exception
  {
    IsNotInitializedException(String s)
    {
      super(s);
    }
  }


  static public class WrongParamTypeException extends Exception
  {
    WrongParamTypeException(String s)
    {
      super(s);
    }
  }


  static public void Init(String configFileName)
  {
    configurationFileName = configFileName;

    int slashpos = configurationFileName.lastIndexOf(File.separatorChar);
    if (slashpos < 0)
      configurationFileDir = "";
    else
      configurationFileDir = configurationFileName.substring(0, slashpos);

    isInitialized = true;
  }

  static public ConfigManager getInstance()
          throws IsNotInitializedException, FactoryConfigurationError, ParserConfigurationException, IOException, SAXException
  {
    if (!isInitialized)
      throw new IsNotInitializedException("Configuration Manager is not initialized. Make ConfigManager.Init(...) call before ConfigManager.getInstance()");

    if (monitor == null)
      monitor = new ConfigManager(configurationFileName);

    return monitor;
  }

  protected ConfigManager(String configFileName)
          throws FactoryConfigurationError, ParserConfigurationException, IOException, SAXException
  {
    InputStream inputStream = new BufferedInputStream(new FileInputStream(configFileName));
    config = ConfigReader.readConfig(inputStream);
  }

  public Config getConfig()
  {
    return config;
  }


  protected class SaveableConfigTree
  {
    private Map sections = new HashMap();
    private Map params = new HashMap();

    public SaveableConfigTree()
    {
    }

    public void putParameter(String name, Object value)
    {
      int dotpos = name.indexOf('.');
      if (dotpos < 0) {
        params.put(name, value);
      } else if (dotpos == 0) {
        putParameter(name.substring(1), value);
      } else {
        SaveableConfigTree sec = (SaveableConfigTree) sections.get(name.substring(0, dotpos));
        if (sec == null) {
          sec = new SaveableConfigTree();
          sections.put(name.substring(0, dotpos), sec);
        }
        sec.putParameter(name.substring(dotpos + 1), value);
      }
    }

    public void write(OutputStream out, String prefix)
            throws WrongParamTypeException, IOException
    {
      writeParams(out, prefix, params);
      writeSections(out, prefix, sections);
    }

    private void writeParams(OutputStream out, String prefix, Map parameters)
            throws WrongParamTypeException, IOException
    {
      for (Iterator i = parameters.keySet().iterator(); i.hasNext();) {
        String paramName = (String) i.next();
        Object paramValue = parameters.get(paramName);
        out.write((prefix + "<param name=\"" + StringEncoderDecoder.encode(paramName) + "\" type=\"").getBytes());
        if (paramValue instanceof String) {
          out.write(("string\">" + ((String) paramValue) + "</param>\n").getBytes());
        } else if (paramValue instanceof Integer) {
          out.write(("int\">" + StringEncoderDecoder.encode(String.valueOf(((Integer) paramValue).longValue())) + "</param>\n").getBytes());
        } else if (paramValue instanceof Long) {
          out.write(("int\">" + StringEncoderDecoder.encode(String.valueOf(((Long) paramValue).longValue())) + "</param>\n").getBytes());
        } else if (paramValue instanceof Boolean) {
          out.write(("bool\">" + StringEncoderDecoder.encode(String.valueOf(((Boolean) paramValue).booleanValue())) + "</param>\n").getBytes());
        } else {
          throw new WrongParamTypeException("unknown type of parameter " + paramName);
        }
      }
    }

    private void writeSections(OutputStream out, String prefix, Map secs)
            throws IOException, WrongParamTypeException
    {
      for (Iterator i = secs.keySet().iterator(); i.hasNext();) {
        String secName = (String) i.next();
        SaveableConfigTree childs = (SaveableConfigTree) secs.get(secName);
        out.write((prefix + "<section name=\"" + StringEncoderDecoder.encode(secName) + "\">\n").getBytes());
        childs.write(out, prefix + "  ");
        out.write((prefix + "</section>\n").getBytes());
      }
    }
  };

  public void save()
          throws IOException, WrongParamTypeException
  {
    org.apache.log4j.Category.getInstance(this.getClass()).debug("SAVE");
    for (Iterator iii = config.params.keySet().iterator(); iii.hasNext();) {
      String name = (String) iii.next();
      org.apache.log4j.Category.getInstance(this.getClass()).debug(name);
    }

    SaveableConfigTree tree = new SaveableConfigTree();
    File tmpFile = File.createTempFile("smsc_config_", ".xml.tmp", new File(configurationFileDir));
    OutputStream out = new FileOutputStream(tmpFile);
    for (Iterator i = config.params.keySet().iterator(); i.hasNext();) {
      String name = (String) i.next();
      tree.putParameter(name, config.params.get(name));
    }
    out.write("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n".getBytes());
    out.write("<!DOCTYPE config SYSTEM \"file://configuration.dtd\">\n\n".getBytes());

    out.write("<config>\n".getBytes());
    tree.write(out, "  ");
    out.write("</config>\n".getBytes());
    out.flush();
    out.close();

    tmpFile.renameTo(new File(configurationFileName));
  }
}
