package mobi.eyeline.util.jsf.components.updatable_content;

import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.webapp.UIComponentELTag;

/**
 * User: artem
 * Date: 28.07.11
 */
public class UpdatableContentTag extends UIComponentELTag {

  private ValueExpression updatePeriod;
  private ValueExpression enabled;

  public void setEnabled(ValueExpression enabled) {
    this.enabled = enabled;
  }

  public void setUpdatePeriod(ValueExpression updatePeriod) {
    this.updatePeriod = updatePeriod;
  }

  @Override
  public void release() {
    super.release();
    updatePeriod = null;
    enabled = null;
  }

  @Override
  public String getComponentType() {
    return "mobi.eyeline.util.jsf.components.updatable_content";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.updatable_content";
  }

  @Override
  protected void setProperties(UIComponent component) {
    super.setProperties(component);

    UpdatableContent c = (UpdatableContent)component;

    if (updatePeriod != null)
      c.setValueExpression("updatePeriod", updatePeriod);

    if (enabled != null)
      c.setValueExpression("enabled", enabled);
  }
}
