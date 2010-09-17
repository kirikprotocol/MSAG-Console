package mobi.eyeline.informer.web.components.buttons_panel;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.faces.component.UIComponent;

/**
 * @author Artem Snopkov
 */
public class ButtonsPanelHandler extends ComponentHandler {
  public ButtonsPanelHandler(ComponentConfig config) {
    super(config);
  }

  @Override
  protected UIComponent createComponent(FaceletContext ctx) {
    return new ButtonsPannel();
  }
}
