package ru.sibinco.calendarsme.engine.timezones.parsers.tags.timezones;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import ru.sibinco.calendarsme.engine.timezones.parsers.TimezonesParser;
import ru.sibinco.calendarsme.engine.timezones.parsers.Parser;
import ru.sibinco.calendarsme.engine.timezones.parsers.tags.Tag;
import ru.sibinco.calendarsme.engine.timezones.parsers.tags.UnknownTag;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class Param extends Tag {

  public static final String QNAME = "param";

  private final String name;

  private String value = null;

  public Param(final Tag parentTag, final Attributes atts, final Parser parser) {
    super(parentTag, parser);
    this.name = atts.getValue("name");
  }

  public void characters(char ch[], int start, int length) throws SAXException {
    value = String.valueOf(ch, start, length);
    if (value != null && value.indexOf(',') > 0)
      value = value.substring(value.indexOf(',') + 1);
  }

  private TimezonesParser getTimezonesParser() {
    return (TimezonesParser)getParser();
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
    if (name != null && value != null) {
      if (!name.equalsIgnoreCase("default_timezone"))
        getTimezonesParser().addTimezone(name, value);
      else
        getTimezonesParser().setDefaultTimezone(value);
    }
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
