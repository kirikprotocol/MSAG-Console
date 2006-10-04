package ru.sibinco.calendarsme.engine.timezones.parsers.tags.routes;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import ru.sibinco.calendarsme.engine.timezones.parsers.Parser;
import ru.sibinco.calendarsme.engine.timezones.parsers.RoutesParser;
import ru.sibinco.calendarsme.engine.timezones.parsers.tags.Tag;
import ru.sibinco.calendarsme.engine.timezones.parsers.tags.UnknownTag;

import java.util.Map;
import java.util.TreeMap;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class SubjectDef extends Tag {

  public static final String QNAME = "subject_def";

  private final String id;
  private final Map masks;

  public SubjectDef(Tag parentTag, Parser parser, Attributes atts) {
    super(parentTag, parser);

    this.id = atts.getValue("id");
    this.masks = new TreeMap();
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
    ((RoutesParser)getParser()).addRoutes(masks);
  }

  public void addMask(final String mask) {
    masks.put(mask, id);
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    if (qName.equals(Mask.QNAME))
      getParser().setCurrentTag(new Mask(this, getParser(), atts));
    else
      getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
