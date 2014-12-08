package ru.sibinco.scag.web.security;

import org.w3c.dom.Document;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;

public class XMLDocumentParser {

  public static Document parse(final Reader input) throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException, NullPointerException{
    if (null == input) throw new NullPointerException("reader is null");
    final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
    final DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(new DtdsEntityResolver());
    final InputSource source = new InputSource(input);
    return builder.parse(source);
  }

  public static Document parse(final InputStream inputStream) throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException, NullPointerException{
    if (null == inputStream) throw new NullPointerException("input stream is null");
    final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
    final DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(new DtdsEntityResolver());
    final InputSource source = new InputSource(inputStream);
    return builder.parse(source);
  }

  private static class DtdsEntityResolver implements EntityResolver {

    private String getDtdName(final String systemId){
      int index = systemId.lastIndexOf('/');
      if (0 > index)
        index = systemId.lastIndexOf('\\');
      if (0 <= index)
        return systemId.substring(index + 1);
      else
        return systemId;
    }

    public InputSource resolveEntity(final String publicId, final String systemId) throws SAXException, IOException{
      if (systemId.endsWith(".dtd")) {
        String filename = "dtds/" + getDtdName(systemId);
        InputStream in = Thread.currentThread().getContextClassLoader().getResourceAsStream(filename);
        if (in != null) return new InputSource(in);
      }
      return null;
    }
  }

}
