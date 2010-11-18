package mobi.eyeline.informer.util.config;

import java.io.*;

/**
 * @author Aleksandr Khalitov
 *         <p/>
 *         Config based on xml file
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
   * Creates new instance of xml config and load it from input stream
   *
   * @param is input stream
   * @throws XmlConfigException If can't load
   */
  public XmlConfig(InputStream is) throws XmlConfigException {
    this();
    load(is);
  }

  /**
   * Creates new instance of xml config and load it from other
   *
   * @param c XmlConfig
   * @throws XmlConfigException If can't load
   */
  public XmlConfig(XmlConfig c) throws XmlConfigException {
    try {
      XmlConfigSection s = (XmlConfigSection) clone();
      sections = s.sections;
      params = s.params;
      name = s.name;
      encoding = c.encoding;
    } catch (CloneNotSupportedException e) {
      throw new XmlConfigException(e);
    }

  }

  /**
   * Load config from file
   *
   * @param file file
   * @throws XmlConfigException If can't load
   */
  public void load(File file) throws XmlConfigException {
    if (file == null)
      throw new XmlConfigException("Config file not specified");

    InputStream is = null;
    try {
      is = new BufferedInputStream(new FileInputStream(file));
      load(is);
    } catch (IOException e) {
      throw new XmlConfigException("Can't load config", e);
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
      }
    }
  }

  /**
   * Load config from input stream
   *
   * @param is input stream
   * @throws XmlConfigException If can't load
   */
  public void load(InputStream is) throws XmlConfigException {
    clear();
    XmlConfigReader.loadConfig(is, this);
  }

  /**
   * Save config
   *
   * @param os output stream
   * @throws XmlConfigException if cant save
   */
  public void save(OutputStream os) throws XmlConfigException {
    try {
      XmlConfigWriter.writeConfig(this, os, encoding == null ? "utf-8" : encoding, "configuration.dtd");
    } catch (Exception e) {
      throw new XmlConfigException(e);
    }
  }

  /**
   * Returns config encoding
   *
   * @return config encoding
   */
  public String getEncoding() {
    return encoding;
  }

  /**
   * Set config encoding
   *
   * @param encoding new encoding
   */
  public void setEncoding(String encoding) {
    this.encoding = encoding;
  }
}
