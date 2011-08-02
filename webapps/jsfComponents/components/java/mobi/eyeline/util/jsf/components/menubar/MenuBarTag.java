package mobi.eyeline.util.jsf.components.menubar;

import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.webapp.UIComponentELTag;

/**
 * User: artem
 * Date: 01.08.11
 */
public class MenuBarTag extends UIComponentELTag  {

  private ValueExpression label;

  public void setLabel(ValueExpression label) {
    this.label = label;
  }

  @Override
  public void release() {
    super.release();
    this.label = null;
  }

  @Override
  protected void setProperties(UIComponent component) {
    super.setProperties(component);

    if (label != null)
      component.setValueExpression("label", label);
  }

  @Override
  public String getComponentType() {
    return "mobi.eyeline.util.jsf.components.menubar";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.menubar";
  }
}
