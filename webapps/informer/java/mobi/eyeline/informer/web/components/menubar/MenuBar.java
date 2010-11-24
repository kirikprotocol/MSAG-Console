package mobi.eyeline.informer.web.components.menubar;

import mobi.eyeline.informer.web.components.EyelineComponent;

import javax.faces.context.FacesContext;

/**
 * @author Artem Snopkov
 */
public class MenuBar extends EyelineComponent {

  private String label;

  public String getLabel() {
    return label;
  }

  public void setLabel(String label) {
    this.label = label;
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[2];
    values[0] = super.saveState(context);
    values[1] = label;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    label = (String) values[1];
  }
}
