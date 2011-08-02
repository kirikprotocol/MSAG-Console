package mobi.eyeline.util.jsf.components.dynamic_table;

import javax.faces.webapp.UIComponentELTag;

/**
 * User: artem
 * Date: 02.08.11
 */
public class DynamicTableTag extends UIComponentELTag {
  @Override
  public String getComponentType() {
    return "mobi.eyeline.util.jsf.components.dynamic_table";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.dynamic_table";
  }
}
