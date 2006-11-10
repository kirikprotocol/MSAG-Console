package ru.sibinco.otasme.engine.smscenters.parser;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;
import ru.sibinco.otasme.engine.smscenters.parser.tags.Tag;
import ru.sibinco.otasme.engine.smscenters.parser.tags.smscenters.Config;
import ru.sibinco.otasme.engine.smscenters.ParseException;
import ru.sibinco.otasme.engine.smscenters.SMSCenter;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import java.io.IOException;
import java.io.InputStream;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.Collection;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class SMSCentersParser extends DefaultHandler implements Parser {

  private Tag currentTag = null;
  private final SortedMap smscentersMap = new TreeMap();
  private String defaultTimezone;

  /**
   * Reads and parses xml with timezones from source.
   * @param source
   * key = timezone name;
   * value = timezone string
   * @throws ru.sibinco.otasme.engine.smscenters.ParseException
   */

  public void parse(InputStream source) throws ParseException {
    SAXParserFactory fact = SAXParserFactory.newInstance();

    try {
      SAXParser saxParser = fact.newSAXParser();
      saxParser.parse(source, this);

    } catch (ParserConfigurationException e) {
      throw new ParseException(e);
    } catch (SAXException e) {
      throw new ParseException(e);
    } catch (IOException e) {
      throw new ParseException(e);
    }
  }

  public void setCurrentTag(final Tag tag) {
    this.currentTag = tag;
  }

  private SMSCenter getTimezone(String timezoneName) {
    SMSCenter tz = (SMSCenter)smscentersMap.get(timezoneName);
    if (tz == null) {
      tz = new SMSCenter(timezoneName);
      smscentersMap.put(timezoneName, tz);
    }
    return tz;
  }

  public void addRoute(final String route, final String timezoneName) {
    getTimezone(timezoneName).addRoute(route);
  }

  public void addMask(final String mask, final String timezoneName) {
    getTimezone(timezoneName).addMask(mask);
  }

  public void setDefaultTimezone(String defaultTimezone) {
    this.defaultTimezone = defaultTimezone;
  }

  public String getDefaultSMSCenter() {
    return defaultTimezone;
  }

  public Collection getTimezones() {
    return smscentersMap.values();
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
    if (qName.equals(Config.QNAME)) {
      currentTag = new Config(null, this);
    } else
      currentTag.startElement(namespaceURI, localName, qName, atts);
  }
}
