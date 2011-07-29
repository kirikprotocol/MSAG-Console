package mobi.eyeline.util.jsf.components.updatable_content;

import javax.el.ValueExpression;
import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import javax.servlet.jsp.tagext.JspTag;

/**
 * @author Aleksandr Khalitov
 */
public class UpdatableContent extends UIPanel {

  private int updatePeriod;
  private boolean enabled = true;

  public int getUpdatePeriod() {
    ValueExpression exp = getValueExpression("updatePeriod");
    if (exp == null)
      return updatePeriod;
    else
      return (Integer)exp.getValue(getFacesContext().getELContext());
  }

  public void setUpdatePeriod(int updatePeriod) {
    this.updatePeriod = updatePeriod;
  }

  public boolean isEnabled() {
    return enabled;
  }

  public void setEnabled(boolean enabled) {
    this.enabled = enabled;
  }

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new UpdatableContentRenderer();
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[3];
    values[0] = super.saveState(context);
    values[1] = updatePeriod;
    values[2] = enabled;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    updatePeriod = (Integer) values[1];
    enabled = (Boolean) values[2];
  }
}
