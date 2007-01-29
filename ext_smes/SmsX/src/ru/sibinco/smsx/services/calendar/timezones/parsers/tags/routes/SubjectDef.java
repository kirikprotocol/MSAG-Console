package ru.sibinco.smsx.services.calendar.timezones.parsers.tags.routes;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import ru.sibinco.smsx.services.calendar.timezones.parsers.RoutesParser;
import ru.sibinco.smsx.utils.xml.Parser;
import ru.sibinco.smsx.utils.xml.Tag;
import ru.sibinco.smsx.utils.xml.UnknownTag;

import java.util.HashSet;
import java.util.Set;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class SubjectDef extends Tag {

  public static final String QNAME = "subject_def";

  private final String id;
  private final Set masks;

  public SubjectDef(Tag parentTag, Parser parser, Attributes atts) {
    super(parentTag, parser);

    this.id = atts.getValue("id");
    this.masks = new HashSet();
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
    ((RoutesParser)getParser()).addMasks(id, masks);
  }

  public void addMask(final String mask) {
    masks.add(mask);
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
