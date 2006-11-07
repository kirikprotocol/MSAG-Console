package ru.sibinco.otasme.engine.template.parser.tags;

import org.xml.sax.Attributes;
import ru.sibinco.otasme.engine.template.Automation;
import ru.sibinco.otasme.engine.template.State;

import java.util.ArrayList;

/**
 * User: artem
 * Date: 25.10.2006
 */

public class AutomationTag extends Tag {

  public static final String QNAME="automation";

  private ArrayList states = new ArrayList();
  private final Automation a;

  public AutomationTag(Tag parentTag, Automation a) {
    super(parentTag);
    this.a = a;
  }


  public Tag startElement(String qname, Attributes atts) {
    if (qname.equalsIgnoreCase(LinksTag.QNAME))
      return new LinksTag(this, states);
    else if (qname.equals(StatesTag.QNAME))
      return new StatesTag(this, a, states, atts);

    return null;
  }

  public Tag endElement() {
    a.setStartState((State)states.get(0));
    return super.endElement();
  }
}
