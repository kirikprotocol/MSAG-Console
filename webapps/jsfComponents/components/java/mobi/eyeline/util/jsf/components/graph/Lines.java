package mobi.eyeline.util.jsf.components.graph;


import javax.el.ValueExpression;
import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class Lines extends UIPanel {

  private List labels;

  private int updatePeriod;

  private int height;

  private int width;

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new LinesRenderer();
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[5];
    values[0] = super.saveState(context);
    values[1] = labels;
    values[2] = updatePeriod;
    values[3] = height;
    values[4] = width;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    labels = (List)values[1];
    updatePeriod = (Integer)values[2];
    height = (Integer)values[3];
    width = (Integer)values[4];
  }

  public int getHeight() {
    ValueExpression exp = getValueExpression("height");
    if (exp == null)
      return height;
    else
      return (Integer) exp.getValue(getFacesContext().getELContext());
  }

  public void setHeight(int height) {
    this.height = height;
  }

  public int getWidth() {
    ValueExpression exp = getValueExpression("width");
    if (exp == null)
      return width;
    else
      return (Integer) exp.getValue(getFacesContext().getELContext());
  }

  public void setWidth(int width) {
    this.width = width;
  }

  public List getLabels() {
    ValueExpression exp = getValueExpression("labels");
    if (exp == null)
      return labels;
    else
      return (List) exp.getValue(getFacesContext().getELContext());
  }

  public void setLabels(List labels) {
    this.labels = labels;
  }

  public Integer getUpdatePeriod() {
    ValueExpression exp = getValueExpression("updatePeriod");
    if (exp == null)
      return updatePeriod;
    else
      return (Integer) exp.getValue(getFacesContext().getELContext());
  }

  public void setUpdatePeriod(Integer updatePeriod) {
    this.updatePeriod = updatePeriod;
  }
}
