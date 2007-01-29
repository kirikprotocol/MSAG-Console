package ru.sibinco.smsx.services.calendar.timezones.parsers.tags.routes;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import ru.sibinco.smsx.utils.xml.Parser;
import ru.sibinco.smsx.utils.xml.Tag;
import ru.sibinco.smsx.utils.xml.UnknownTag;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class Routes extends Tag {

  public static final String QNAME = "routes";

  public Routes(Tag parentTag, Parser parser) {
    super(parentTag, parser);
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    if (qName.equals(SubjectDef.QNAME))
      getParser().setCurrentTag(new SubjectDef(this, getParser(), atts));
    else
      getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
