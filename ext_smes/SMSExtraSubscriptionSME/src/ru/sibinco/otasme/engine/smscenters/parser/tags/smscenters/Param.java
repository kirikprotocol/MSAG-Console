package ru.sibinco.otasme.engine.smscenters.parser.tags.smscenters;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import ru.sibinco.otasme.engine.smscenters.parser.SMSCentersParser;
import ru.sibinco.otasme.engine.smscenters.parser.Parser;
import ru.sibinco.otasme.engine.smscenters.parser.tags.Tag;
import ru.sibinco.otasme.engine.smscenters.parser.tags.UnknownTag;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class Param extends Tag {

  public static final String QNAME = "param";

  private final String name;
  private final String sectionName;

  private String value = null;

  public Param(final Tag parentTag, final String sectionName, final Attributes atts, final Parser parser) {
    super(parentTag, parser);
    this.name = atts.getValue("name");
    this.sectionName = sectionName;
  }

  public void characters(char ch[], int start, int length) throws SAXException {
    value = String.valueOf(ch, start, length);
    if (value != null && value.indexOf(',') > 0)
      value = value.substring(value.indexOf(',') + 1);
  }

  private SMSCentersParser getTimezonesParser() {
    return (SMSCentersParser)getParser();
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
    if (name != null && value != null) {
      if (sectionName.equalsIgnoreCase("masks")) {
        getTimezonesParser().addMask(name, value);
      } else if (!name.equalsIgnoreCase("default_smscenter")) {
        getTimezonesParser().addRoute(name, value);
      } else
        getTimezonesParser().setDefaultTimezone(value);
    }
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
