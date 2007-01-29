package ru.sibinco.smsx.services.redirector.redirects.parsers.tags;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import ru.sibinco.smsx.utils.xml.Parser;
import ru.sibinco.smsx.utils.xml.Tag;
import ru.sibinco.smsx.utils.xml.UnknownTag;

import java.util.List;

/**
 * User: artem
 * Date: 24.01.2007
 */
public class Redirect extends Tag{

  public static final String QNAME = "redirect";


  public Redirect(Tag parentTag, Parser parser, Attributes atts, List redirects) {
    super(parentTag, parser);
    redirects.add(new ru.sibinco.smsx.services.redirector.redirects.Redirect(atts.getValue("regex"), atts.getValue("address")));
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
