package mobi.eyeline.informer.web.components.graph;

import mobi.eyeline.informer.web.components.EyelineComponent;

import javax.faces.context.FacesContext;

/**
 * @author Aleksandr Khalitov
 */
public class LineChart extends EyelineComponent{

  private int[][] values;

  private String color;

  private boolean shade;

  public boolean getShade() {
    return shade;
  }

  public void setShade(boolean shade) {
    this.shade = shade;
  }

  public String getColor() {
    return color;
  }

  public void setColor(String color) {
    this.color = color;
  }

  public int[][] getValues() {
    return values;
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
