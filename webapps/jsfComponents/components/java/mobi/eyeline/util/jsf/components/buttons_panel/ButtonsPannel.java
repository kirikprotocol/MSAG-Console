package mobi.eyeline.util.jsf.components.buttons_panel;

import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import javax.servlet.jsp.tagext.JspTag;

/**
 * @author Artem Snopkov
 */
public class ButtonsPannel extends UIPanel implements JspTag {
  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.buttons_panel";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new ButtonsPanelRenderer();
  }
}
