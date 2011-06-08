package mobi.eyeline.util.jsf.components.graph;

import mobi.eyeline.util.jsf.components.EyelineComponent;

import javax.faces.context.FacesContext;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class Lines extends EyelineComponent {

  private List labels;

  private int updatePeriod;

  private int height;

  private int width;

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
    return height;
  }

  public void setHeight(int height) {
    this.height = height;
  }

  public int getWidth() {
    return width;
  }

  public void setWidth(int width) {
    this.width = width;
  }

  public List getLabels() {
    return labels;
  }

  public void setLabels(List labels) {
    this.labels = labels;
  }

  public Integer getUpdatePeriod() {
    return updatePeriod;
  }

  public void setUpdatePeriod(Integer updatePeriod) {
    this.updatePeriod = updatePeriod;
  }
}
