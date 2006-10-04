package ru.sibinco.calendarsme.engine.timezones.parsers;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;
import ru.sibinco.calendarsme.engine.timezones.parsers.tags.Tag;
import ru.sibinco.calendarsme.engine.timezones.parsers.tags.timezones.Config;
import ru.sibinco.calendarsme.engine.timezones.ParseException;
import ru.sibinco.calendarsme.engine.timezones.parsers.Parser;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import java.io.IOException;
import java.io.InputStream;
import java.util.SortedMap;
import java.util.TreeMap;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class TimezonesParser extends DefaultHandler implements Parser {

  /**
   * Reads and parses xml with timezones from source.
   * @param source
   * @return SortedMap with structure:
   * key = timezone name;
   * value = timezone string
   * @throws ParseException
   */
  public static SortedMap parse(InputStream source) throws ParseException {
    SAXParserFactory fact = SAXParserFactory.newInstance();

    try {
      SAXParser saxParser = fact.newSAXParser();
      final TimezonesParser parser = new TimezonesParser();
      saxParser.parse(source, parser);

      final SortedMap result =  parser.getTimezonesMap();
      result.put("defaultTimezone", parser.getDefaultTimezone());
      return result;
    } catch (ParserConfigurationException e) {
      throw new ParseException(e);
    } catch (SAXException e) {
      throw new ParseException(e);
    } catch (IOException e) {
      throw new ParseException(e);
    }
  }

  private Tag currentTag = null;
  private final SortedMap timezonesMap;
  private String defaultTimezone;

  public TimezonesParser() {
    this.timezonesMap = new TreeMap();
  }

  public void setCurrentTag(final Tag tag) {
    this.currentTag = tag;
  }

  public void addTimezone(final String name, final String value) {
    timezonesMap.put(name, value);
  }

  public void setDefaultTimezone(String defaultTimezone) {
    this.defaultTimezone = defaultTimezone;
  }

  public String getDefaultTimezone() {
    return defaultTimezone;
  }

  public SortedMap getTimezonesMap() {
    return timezonesMap;
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
