package mobi.eyeline.util.jsf.components.menubar;

import javax.faces.webapp.UIComponentELTag;

/**
 * User: artem
 * Date: 01.08.11
 */
public class MenuBarItemTag extends UIComponentELTag  {
  @Override
  public String getComponentType() {
    return "mobi.eyeline.util.jsf.components.menubar.item";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.menubar";
  }
}
