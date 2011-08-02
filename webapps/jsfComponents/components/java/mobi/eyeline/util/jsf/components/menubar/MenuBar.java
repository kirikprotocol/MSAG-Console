package mobi.eyeline.util.jsf.components.menubar;

import mobi.eyeline.util.jsf.components.EyelineComponent;

import javax.el.ValueExpression;
import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * @author Artem Snopkov
 */
public class MenuBar extends UIPanel {

  private String label;

  public String getLabel() {
    ValueExpression exp = getValueExpression("label");
    if (exp == null)
      return label;
    else
      return (String) exp.getValue(getFacesContext().getELContext());
  }

  public void setLabel(String label) {
    this.label = label;
  }

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new MenuBarRenderer();
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[2];
    values[0] = super.saveState(context);
    values[1] = label;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    label = (String) values[1];
  }
}
