package mobi.eyeline.util.jsf.components.graph;


import javax.el.ValueExpression;
import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * @author Aleksandr Khalitov
 */
public class LineChart extends UIPanel {

  private int[][] values;

  private String color;

  private boolean shade;

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new LinesRenderer();
  }

  public boolean getShade() {
    ValueExpression exp = getValueExpression("shade");
    if (exp == null)
      return shade;
    else
      return (Boolean) exp.getValue(getFacesContext().getELContext());
  }

  public void setShade(boolean shade) {
    this.shade = shade;
  }

  public String getColor() {
    ValueExpression exp = getValueExpression("color");
    if (exp == null)
      return color;
    else
      return (String) exp.getValue(getFacesContext().getELContext());
  }

  public void setColor(String color) {
    this.color = color;
  }

  public int[][] getValues() {
    ValueExpression exp = getValueExpression("values");
    if (exp == null)
      return values;
    else
      return (int[][]) exp.getValue(getFacesContext().getELContext());
  }

  public void setValues(int[][] values) {
    this.values = values;
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[4];
    values[0] = super.saveState(context);
    values[1] = this.values;
    values[2] = color;
    values[3] = shade;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    this.values = (int[][])values[1];
    color = (String)values[2];
    shade = (Boolean)values[3];
  }
}
