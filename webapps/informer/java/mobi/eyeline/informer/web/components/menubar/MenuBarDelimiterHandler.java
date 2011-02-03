package mobi.eyeline.informer.web.components.menubar;

import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

/**
 * User: artem
 * Date: 03.02.11
 */
public class MenuBarDelimiterHandler extends ComponentHandler {
  public MenuBarDelimiterHandler(ComponentConfig config) {
    super(config);
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    return new MenuBarDelimiter();
  }
}
