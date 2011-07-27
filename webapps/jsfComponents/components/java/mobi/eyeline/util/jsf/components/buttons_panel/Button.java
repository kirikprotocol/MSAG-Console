package mobi.eyeline.util.jsf.components.buttons_panel;

import mobi.eyeline.util.jsf.components.EyelineComponent;

import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import javax.servlet.jsp.tagext.JspTag;

/**
 * @author Artem Snopkov
 */
public class Button extends UIPanel implements JspTag {

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.buttons_panel.button";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new ButtonsPanelRenderer();
  }
}
