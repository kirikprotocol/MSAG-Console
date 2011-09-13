package mobi.eyeline.util.jsf.components.progress_bar;

import javax.el.ValueExpression;
import javax.faces.component.UIOutput;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * User: artem
 * Date: 29.07.11
 */
public class ProgressBar extends UIOutput {

  private int current;
  private int maximum;

  public void setCurrent(int current) {
    this.current = current;
  }

  public int getCurrent() {
    ValueExpression exp = getValueExpression("current");
    if (exp == null)
      return current;
    else
      return (Integer)exp.getValue(getFacesContext().getELContext());
  }

  public void setMaximum(int maximum) {
    this.maximum = maximum;
  }

  public int getMaximum() {
    ValueExpression exp = getValueExpression("maximum");
    if (exp == null)
      return maximum;
    else
      return (Integer)exp.getValue(getFacesContext().getELContext());
  }

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new ProgressBarRenderer();
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[3];
    values[0] = super.saveState(context);
    values[1] = current;
    values[2] = maximum;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    current = (Integer) values[1];
    maximum = (Integer) values[2];
  }
}
