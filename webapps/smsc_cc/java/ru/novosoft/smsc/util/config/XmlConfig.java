package ru.novosoft.smsc.util.config;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerException;
import java.io.*;

/**
 * author: alkhal
 *
 * Config based on xml file
 */

@SuppressWarnings({"EmptyCatchBlock"})
public class XmlConfig extends XmlConfigSection {


  private String encoding = null;

  /**
   * Creates new instance of xml config
   */
  public XmlConfig() {
  }

  /**
   * Creates new instance of xml config and load it from file
   * @param file Config file
   * @throws ConfigException If can't load
   */
  public XmlConfig(File file) throws ConfigException{
    this();
    load(file);
  }

  /**
   * Creates new instance of xml config and load it from input stream
   * @param is input stream
   * @throws ConfigException If can't load
   */
  public XmlConfig(InputStream is) throws ConfigException{
    this();
    load(is);
  }

  /**
   * Creates new instance of xml config and load it from other
   * @param c XmlConfig
   * @throws ConfigException If can't load
   */
  public XmlConfig(XmlConfig c) throws ConfigException{
    try{
      XmlConfigSection s =  (XmlConfigSection)clone();
      sections = s.sections;
      params = s.params;
      name = s.name;
      encoding = c.encoding;
    }catch (CloneNotSupportedException e){
      throw new ConfigException(e);
    }

  }

  /**
   * Load config from file
   * @param file file
   * @throws ConfigException If can't load
   */
  public void load(File file) throws ConfigException {
    if (file == null)
      throw new ConfigException("Config file not specified");

    InputStream is = null;
    try {
      is = new BufferedInputStream(new FileInputStream(file));
      load(is);
    } catch (IOException e) {
      throw new ConfigException("Can't load config", e);
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {}
    }
  }

  /**
   * Load config from input stream
   * @param is input stream
   * @throws ConfigException If can't load
   */
  public void load(InputStream is) throws ConfigException {
    clear();
    XmlConfigReader.loadConfig(is, this);
  }

  /**
   * Save config
   * @param os output stream
   * @throws ConfigException if cant save
   */
  public void save(OutputStream os) throws ConfigException {
    try{
      XmlConfigWriter.writeConfig(this, os, encoding == null ? "utf-8" : encoding ,"configuration.dtd");
    }catch (Exception e) {
      throw new ConfigException(e);
    }
  }

  /**
   * Returns config encoding
   * @return config encoding
   */
  public String getEncoding() {
    return encoding;
  }

  /**
   * Set config encoding
   * @param encoding new encoding
   */
  public void setEncoding(String encoding) {
    this.encoding = encoding;
  }
}
