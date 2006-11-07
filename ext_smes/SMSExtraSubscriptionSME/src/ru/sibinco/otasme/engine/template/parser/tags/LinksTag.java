package ru.sibinco.otasme.engine.template.parser.tags;

import org.xml.sax.Attributes;

import java.util.ArrayList;

/**
 * User: artem
 * Date: 25.10.2006
 */

public class LinksTag extends Tag {

  public static final String QNAME="links";

  private final ArrayList states;

  public LinksTag(Tag parentTag, ArrayList states) {
    super(parentTag);
    this.states = states;
  }

  public Tag startElement(String qname, Attributes atts) {
    if (qname.equalsIgnoreCase(BaseStateTag.QNAME))
      return new BaseStateTag(this, states, atts);

    return null;
  }
}
