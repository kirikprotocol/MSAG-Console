package mobi.eyeline.util.jsf.components.menubar;

import javax.faces.component.UIOutput;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * @author Aleksandr Khalitov
 */
public class MenuBarDelimiter extends UIOutput {
  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new MenuBarRenderer();
  }
}
