package mobi.eyeline.util.jsf.components.tabs;

import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * User: artem
 * Date: 21.11.11
 */
public class Tabs extends UIPanel  {
  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.tabs";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new TabsRenderer();
  }
}
