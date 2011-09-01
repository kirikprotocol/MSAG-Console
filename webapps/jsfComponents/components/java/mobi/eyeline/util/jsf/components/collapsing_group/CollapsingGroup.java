package mobi.eyeline.util.jsf.components.collapsing_group;

import javax.el.ValueExpression;
import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import javax.servlet.jsp.tagext.JspTag;

/**
 * @author Artem Snopkov
 */
public class CollapsingGroup extends UIPanel implements JspTag {

  private String label;
  private boolean opened;

  public String getLabel() {
    ValueExpression exp = getValueExpression("label");
    if (exp == null) {
      return label;
    } else
      return (String) exp.getValue(getFacesContext().getELContext());
  }

  public void setLabel(String label) {
    this.label = label;
  }

  public boolean isOpened() {
    return opened;
  }

  public void setOpened(boolean opened) {
    this.opened = opened;
  }

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.collapsing_group";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new CollapsingGroupRenderer();
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[3];
    values[0] = super.saveState(context);
    values[1] = opened;
    values[2] = label;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    opened = (Boolean) values[1];
    label = (String) values[2];
  }
}
