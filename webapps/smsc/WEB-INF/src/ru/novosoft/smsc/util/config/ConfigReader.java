/*
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 26.04.2002
 * Time: 15:59:07
 * To change template for new class use
 * Code Style | Class Templates options (Tools | IDE Options).
 */
package ru.novosoft.smsc.util.config;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.io.InputStream;


public class ConfigReader
{
  protected static Category logger = Category.getInstance(ConfigReader.class);

  public static Config readConfig(InputStream inputStream)
          throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException
  {
    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
    DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(new ConfigEntityResolver());
    InputSource source = new InputSource(inputStream);
    if (inputStream == null)
      logger.error("Config file not found");
    Document doc = builder.parse(source);
    return new Config(doc);
  }
}
