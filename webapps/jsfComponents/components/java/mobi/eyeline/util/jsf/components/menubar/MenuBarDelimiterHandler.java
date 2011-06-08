package mobi.eyeline.util.jsf.components.menubar;

import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

/**
 * @author Aleksandr Khalitov
 */
public class MenuBarDelimiterHandler extends ComponentHandler {
  public MenuBarDelimiterHandler(ComponentConfig config) {
    super(config);
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    return new MenuBarDelimiter();
  }
}
