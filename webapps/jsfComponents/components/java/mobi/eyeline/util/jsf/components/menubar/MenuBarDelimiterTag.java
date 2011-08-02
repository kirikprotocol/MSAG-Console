package mobi.eyeline.util.jsf.components.menubar;

import javax.faces.webapp.UIComponentELTag;

/**
 * User: artem
 * Date: 01.08.11
 */
public class MenuBarDelimiterTag extends UIComponentELTag {
  @Override
  public String getComponentType() {
    return "mobi.eyeline.util.jsf.components.menubar.delimiter";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.menubar";
  }
}
