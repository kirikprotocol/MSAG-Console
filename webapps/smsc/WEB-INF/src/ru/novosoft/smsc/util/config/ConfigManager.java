/*
 * Created by igork
 * Date: Mar 12, 2002
 * Time: 2:53:22 PM
 */
package ru.novosoft.smsc.util.config;

import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.FileInputStream;
import java.io.IOException;

public class ConfigManager
{
  static protected boolean isInitialized = false;
  static protected String configurationFileName = "config.xml";
  static protected ConfigManager monitor = null;

  protected Config config = null;

  static public class IsNotInitialized extends Exception
  {
    IsNotInitialized(String s)
    {
      super(s);
    }
  }

  static public void Init(String configFileName)
  {
    configurationFileName = configFileName;
    isInitialized = true;
  }

  static public ConfigManager getInstance()
          throws IsNotInitialized, FactoryConfigurationError, ParserConfigurationException, IOException, SAXException
  {
    if (!isInitialized)
      throw new IsNotInitialized("Configuration Manager is not initialized. Make ConfigManager.Init(...) call before ConfigManager.getInstance()");

    if (monitor == null)
      monitor = new ConfigManager(configurationFileName);

    return monitor;
  }

  protected ConfigManager(String configFileName)
          throws FactoryConfigurationError, ParserConfigurationException, IOException, SAXException
  {
    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
    DocumentBuilder builder = factory.newDocumentBuilder();
    FileInputStream inputStream = new FileInputStream(configFileName);
    InputSource source = new InputSource(inputStream);
    Document doc = builder.parse(source);
    config = new Config(doc);
  }

  public Config getConfig()
  {
    return config;
  }
}
