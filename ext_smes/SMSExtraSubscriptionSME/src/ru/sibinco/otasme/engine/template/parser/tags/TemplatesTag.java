package ru.sibinco.otasme.engine.template.parser.tags;

import org.xml.sax.Attributes;
import ru.sibinco.otasme.engine.template.Templates;

/**
 * User: artem
 * Date: 25.10.2006
 */

public class TemplatesTag extends Tag {

  public static final String QNAME = "templates";

  private final Templates templates;

  public TemplatesTag(Tag parentTag, Templates templates) {
    super(parentTag);
    this.templates = templates;
  }

  public Tag startElement(String qname, Attributes atts) {
    if (MacroRegionTag.QNAME.equalsIgnoreCase(qname))
      return new MacroRegionTag(this, templates, atts);

    return null;
  }
}
