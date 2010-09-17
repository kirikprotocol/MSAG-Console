package mobi.eyeline.informer.web.components.collapsing_group;

import mobi.eyeline.informer.web.components.EyelineComponent;

import javax.faces.context.FacesContext;

/**
 * @author Artem Snopkov
 */
public class CollapsingGroup extends EyelineComponent {

  private String label;
  private boolean opened;

  public String getLabel() {
    return label;
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

  public Object saveState(FacesContext context) {
    Object[] values = new Object[5];
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
