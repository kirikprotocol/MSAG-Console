package mobi.eyeline.util.jsf.components.buttons_panel;

import javax.faces.component.UIComponentBase;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * @author Artem Snopkov
 */
public class Space extends UIComponentBase {
  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.buttons_panel.space";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new ButtonsPanelRenderer();
  }
}
