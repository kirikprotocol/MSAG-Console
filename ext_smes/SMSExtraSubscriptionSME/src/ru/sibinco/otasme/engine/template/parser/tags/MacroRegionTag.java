package ru.sibinco.otasme.engine.template.parser.tags;

import org.xml.sax.Attributes;
import ru.sibinco.otasme.engine.template.Automation;
import ru.sibinco.otasme.engine.template.MacroRegion;
import ru.sibinco.otasme.engine.template.Templates;

/**
 * User: artem
 * Date: 25.10.2006
 */

public class MacroRegionTag extends Tag {

  public final static String QNAME="MacroRegion";

  private final MacroRegion macroRegion;
  private final Automation automation;

  public MacroRegionTag(Tag parentTag, Templates templates, Attributes atts) {
    super(parentTag);
    macroRegion = new MacroRegion(atts.getValue("name"), atts.getValue("number"));
    templates.addMacroRegion(macroRegion);
    automation = new Automation();
    macroRegion.setAutomation(automation);
  }

  public Tag startElement(String qname, Attributes atts) {
    if (qname.equalsIgnoreCase(AutomationTag.QNAME))
      return new AutomationTag(this, automation);

    return null;
  }
}
