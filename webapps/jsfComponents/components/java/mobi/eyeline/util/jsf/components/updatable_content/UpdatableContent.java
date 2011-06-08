package mobi.eyeline.util.jsf.components.updatable_content;

import mobi.eyeline.util.jsf.components.EyelineComponent;

import javax.faces.context.FacesContext;

/**
 * @author Aleksandr Khalitov
 */
public class UpdatableContent extends EyelineComponent {

  private int updatePeriod;
  private boolean enabled = true;

  public int getUpdatePeriod() {
    return updatePeriod;
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
