package mobi.eyeline.util.jsf.components.menubar;

import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * @author Artem Snopkov
 */
public class MenuBarItem extends UIPanel {

  private boolean hasDelimiter;

  public boolean hasDelimiter() {
    return hasDelimiter;
  }

  public void setHasDelimiter(boolean hasDelimiter) {
    this.hasDelimiter = hasDelimiter;
  }

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new MenuBarRenderer();
  }
}
