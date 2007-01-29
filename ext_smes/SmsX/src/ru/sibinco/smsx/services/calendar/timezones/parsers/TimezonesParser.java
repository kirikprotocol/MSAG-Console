package ru.sibinco.smsx.services.calendar.timezones.parsers;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;
import ru.sibinco.smsx.services.calendar.timezones.Timezone;
import ru.sibinco.smsx.services.calendar.timezones.parsers.tags.timezones.Config;
import ru.sibinco.smsx.utils.xml.ParseException;
import ru.sibinco.smsx.utils.xml.Parser;
import ru.sibinco.smsx.utils.xml.Tag;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import java.io.IOException;
import java.io.InputStream;
import java.util.Collection;
import java.util.SortedMap;
import java.util.TreeMap;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class TimezonesParser extends DefaultHandler implements Parser {

  private Tag currentTag = null;
  private final SortedMap timezonesMap = new TreeMap();
  private String defaultTimezone;

  /**
   * Reads and parses xml with timezones from source.
   * @param source
   * key = timezone name;
   * value = timezone string
   * @throws ru.sibinco.smsx.utils.xml.ParseException
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

  private Timezone getTimezone(String timezoneName) {
    Timezone tz = (Timezone)timezonesMap.get(timezoneName);
    if (tz == null) {
      tz = new Timezone(timezoneName);
      timezonesMap.put(timezoneName, tz);
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

  public String getDefaultTimezone() {
    return defaultTimezone;
  }

  public Collection getTimezones() {
    return timezonesMap.values();
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
    } else if (currentTag != null)
      currentTag.startElement(namespaceURI, localName, qName, atts);
  }
}
