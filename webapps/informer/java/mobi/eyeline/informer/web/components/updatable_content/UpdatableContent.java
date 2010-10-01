package mobi.eyeline.informer.web.components.updatable_content;

import mobi.eyeline.informer.web.components.EyelineComponent;

import javax.faces.context.FacesContext;

/**
 * @author Aleksandr Khalitov
 */
public class UpdatableContent extends EyelineComponent{

  private int updatePeriod;

  public int getUpdatePeriod() {
    return updatePeriod;  
  }

  public void setUpdatePeriod(int updatePeriod) {
    this.updatePeriod = updatePeriod;
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[2];
    values[0] = super.saveState(context);
    values[1] = updatePeriod;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    updatePeriod = (Integer) values[1];
  }
}