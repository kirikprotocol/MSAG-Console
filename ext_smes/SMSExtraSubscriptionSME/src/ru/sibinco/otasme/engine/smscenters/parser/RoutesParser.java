package ru.sibinco.otasme.engine.smscenters.parser;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;
import ru.sibinco.otasme.engine.smscenters.parser.tags.Tag;
import ru.sibinco.otasme.engine.smscenters.parser.tags.routes.Routes;
import ru.sibinco.otasme.engine.smscenters.ParseException;
import ru.sibinco.otasme.engine.smscenters.SMSCenter;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import java.io.IOException;
import java.io.InputStream;
import java.util.Collection;
import java.util.Iterator;
import java.util.Set;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class RoutesParser extends DefaultHandler implements Parser {

  /**
   * Reads and parses xml with smscenters from source.
   * @param source
   * key = phone mask like +7??????????;
   * value = timezone name
   * @throws ParseException
   */
  public static void parse(InputStream source, Collection timezones) throws ParseException {
    SAXParserFactory fact = SAXParserFactory.newInstance();

    try {
      SAXParser saxParser = fact.newSAXParser();
      final RoutesParser parser = new RoutesParser(timezones);
      saxParser.parse(source, parser);

    } catch (ParserConfigurationException e) {
      throw new ParseException(e);
    } catch (SAXException e) {
      throw new ParseException(e);
    } catch (IOException e) {
      throw new ParseException(e);
    }
  }

  private Tag currentTag = null;
  private final Collection smscenters;

  public RoutesParser(Collection smscenters) {
    this.smscenters = smscenters;
  }

  public void setCurrentTag(final Tag tag) {
    this.currentTag = tag;
  }

  private SMSCenter getTimezoneByRoute(final String routeName) {
    for (Iterator iterator = smscenters.iterator(); iterator.hasNext();) {
      SMSCenter tz =  (SMSCenter)iterator.next();
      if (tz.hasRoute(routeName))
        return tz;
    }
    return null;
  }

  public void addMasks(final String routeName, final Set masks) {
    final SMSCenter tz = getTimezoneByRoute(routeName);
    if (tz != null) {
      for (Iterator iterator = masks.iterator(); iterator.hasNext();) {
        String mask =  (String)iterator.next();
        tz.addMask(mask);
      }
    }
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
    if (qName.equals(Routes.QNAME))
      currentTag = new Routes(null, this);
    else
      currentTag.startElement(namespaceURI, localName, qName, atts);

  }
}
