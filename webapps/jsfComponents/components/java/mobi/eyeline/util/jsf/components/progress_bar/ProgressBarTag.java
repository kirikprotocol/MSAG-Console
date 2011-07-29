package mobi.eyeline.util.jsf.components.progress_bar;

import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.webapp.UIComponentELTag;

/**
 * User: artem
 * Date: 29.07.11
 */
public class ProgressBarTag extends UIComponentELTag {

  private ValueExpression current;
  private ValueExpression maximum;

  public void setMaximum(ValueExpression maximum) {
    this.maximum = maximum;
  }

  public void setCurrent(ValueExpression current) {
    this.current = current;
  }

  @Override
  public void release() {
    super.release();
    current = null;
    maximum = null;
  }

  @Override
  protected void setProperties(UIComponent component) {
    super.setProperties(component);

    ProgressBar c = (ProgressBar)component;

    if (current != null)
      c.setValueExpression("current", current);

    if (maximum != null)
      c.setValueExpression("maximum", maximum);
  }

  @Override
  public String getComponentType() {
    return "mobi.eyeline.util.jsf.components.progress_bar";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.progress_bar";
  }
}
