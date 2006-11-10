package ru.sibinco.otasme.engine.smscenters.parser.tags;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import ru.sibinco.otasme.engine.smscenters.parser.Parser;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class UnknownTag extends Tag{
  public UnknownTag(Tag parentTag, Parser parser) {
    super(parentTag, parser);
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
