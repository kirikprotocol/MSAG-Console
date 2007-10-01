package ru.sibinco.smsx.engine.smpphandler.redirects;

import com.eyeline.sme.utils.ds.DataSourceException;
import com.eyeline.sme.utils.xml.EndOfXMLTreeException;
import com.eyeline.sme.utils.xml.reader.sax.AbstractParser;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Collection;
import java.util.LinkedList;

/**
 * User: artem
 * Date: 24.01.2007
 */
class RedirectsDataSource {

  private Collection redirects = new LinkedList();
  private final String file;

  RedirectsDataSource(String file) {
    this.file = file;
  }

  public Collection getRedirects() throws DataSourceException {
    return parseFile(file);
  }

  private Collection parseFile(String fileName) throws DataSourceException {
    InputStream fis = null;
    try {

      fis = new FileInputStream(fileName);
      redirects  = parse(fis);

      return redirects;

    } catch (Exception e) {
      throw new DataSourceException(e);
    } finally {
      try {
        if (fis != null)
          fis.close();
      } catch (IOException e) {
      }
    }
  }

  private static Collection parse(InputStream source) throws DataSourceException {
    SAXParserFactory fact = SAXParserFactory.newInstance();

    final Collection result = new LinkedList();
    try {

      SAXParser saxParser = fact.newSAXParser();
      saxParser.parse(source, new Handler(result));

    } catch (EndOfXMLTreeException e) {
    } catch (ParserConfigurationException e) {
      throw new DataSourceException(e);
    } catch (SAXException e) {
      throw new DataSourceException(e);
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
    return result;
  }

  private static class Handler extends AbstractParser {
    private final Collection redirectsList;

    Handler(Collection redirects) {
      this.redirectsList = redirects;
    }

    public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
      if (qName.equals(RedirectsTag.QNAME)) {
        currentTag = new RedirectsTag(null, this, redirectsList);
      } else if (currentTag != null)
        currentTag.startElement(namespaceURI, localName, qName, atts);
    }
  }

}
