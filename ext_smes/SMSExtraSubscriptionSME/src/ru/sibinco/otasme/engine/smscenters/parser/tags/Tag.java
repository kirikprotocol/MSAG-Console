package ru.sibinco.otasme.engine.smscenters.parser.tags;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import ru.sibinco.otasme.engine.smscenters.parser.Parser;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public abstract class Tag  {

  private final Tag parentTag;
  private final Parser parser;

  public Tag(final Tag parentTag, final Parser parser) {
    this.parentTag = parentTag;
    this.parser = parser;
  }

  protected Parser getParser() {
    return parser;
  }

  protected Tag getParentTag() {
    return parentTag;
  }

  public abstract void endElement(String namespaceURI, String localName, String qName) throws SAXException;

  public abstract void characters(char ch[], int start, int length) throws SAXException;

  public abstract void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException;

  public final void doEndElement(String namespaceURI, String localName, String qName) throws SAXException {
    endElement(namespaceURI, localName, qName);
    parser.setCurrentTag(parentTag);
  }

}
