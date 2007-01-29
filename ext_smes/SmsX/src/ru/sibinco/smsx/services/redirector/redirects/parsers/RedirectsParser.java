package ru.sibinco.smsx.services.redirector.redirects.parsers;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;
import ru.sibinco.smsx.services.redirector.redirects.parsers.tags.Redirects;
import ru.sibinco.smsx.utils.xml.ParseException;
import ru.sibinco.smsx.utils.xml.Parser;
import ru.sibinco.smsx.utils.xml.Tag;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

/**
 * User: artem
 * Date: 24.01.2007
 */
public class RedirectsParser extends DefaultHandler implements Parser{

  private Tag currentTag = null;
  private final List redirects = new ArrayList();

  public static List parseFile(String fileName) {
    InputStream fis = null;
    try {
      fis = ClassLoader.getSystemResourceAsStream(fileName);
      final RedirectsParser parser = new RedirectsParser();
      return parser.parse(fis);

    } catch (Exception e) {
      throw new ParseException(e);
    } finally {
      try {
        if (fis != null)
          fis.close();
      } catch (IOException e) {
      }
    }
  }

  public List parse(InputStream source) throws ParseException {
    SAXParserFactory fact = SAXParserFactory.newInstance();

    try {
      SAXParser saxParser = fact.newSAXParser();
      saxParser.parse(source, this);

      return redirects;

    } catch (ParserConfigurationException e) {
      throw new ParseException(e);
    } catch (SAXException e) {
      throw new ParseException(e);
    } catch (IOException e) {
      throw new ParseException(e);
    }
  }

  public void setCurrentTag(Tag tag) {
    this.currentTag = tag;
  }

  public void characters(char ch[], int start, int length) throws SAXException {
    if (currentTag != null)
      currentTag.characters(ch, start, length);
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
    if (currentTag!=null)
      currentTag.doEndElement(namespaceURI, localName, qName);
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    if (qName.equals(Redirects.QNAME)) {
      currentTag = new Redirects(null, this, redirects);
    } else if (currentTag != null)
      currentTag.startElement(namespaceURI, localName, qName, atts);
  }
}
