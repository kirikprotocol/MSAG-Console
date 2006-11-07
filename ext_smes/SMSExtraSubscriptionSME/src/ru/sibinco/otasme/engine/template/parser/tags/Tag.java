package ru.sibinco.otasme.engine.template.parser.tags;

import org.xml.sax.Attributes;

/**
 * User: artem
 * Date: 25.10.2006
 */

public abstract class Tag {

  private final Tag parentTag;

  public Tag(Tag parentTag) {
    this.parentTag = parentTag;
  }

  public abstract Tag startElement(String qname, Attributes atts);

  public Tag endElement() {
    return parentTag;
  };
}
