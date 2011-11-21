package mobi.eyeline.util.jsf.components.tabs;

import javax.el.ValueExpression;
import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * User: artem
 * Date: 21.11.11
 */
public class Tab extends UIPanel {

  private String label;

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.tab";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new TabsRenderer();
  }

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

  public Object saveState(FacesContext context) {
    Object[] values = new Object[3];
    values[0] = super.saveState(context);
    values[1] = label;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    label        = (String) values[1];
  }
}
