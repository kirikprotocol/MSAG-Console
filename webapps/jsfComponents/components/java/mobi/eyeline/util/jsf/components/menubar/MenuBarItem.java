package mobi.eyeline.util.jsf.components.menubar;

import mobi.eyeline.util.jsf.components.EyelineComponent;

import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * @author Artem Snopkov
 */
public class MenuBarItem extends UIPanel {
  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new MenuBarRenderer();
  }
}
