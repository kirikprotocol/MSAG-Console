package mobi.eyeline.util.jsf.components.collapsing_group;

import mobi.eyeline.util.jsf.components.EyelineComponent;

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

  public void processDecodes(javax.faces.context.FacesContext context) {
    CollapsingGroupRenderer.decodeGroup(context, this);
    super.processDecodes(context);
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
