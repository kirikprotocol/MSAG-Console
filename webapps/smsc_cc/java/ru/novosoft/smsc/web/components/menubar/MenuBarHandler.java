package ru.novosoft.smsc.web.components.menubar;

import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

/**
 * @author Artem Snopkov
 */
public class MenuBarHandler extends ComponentHandler {

  private final TagAttribute label;

  public MenuBarHandler(ComponentConfig config) {
    super(config);

    label = getRequiredAttribute("label");
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    MenuBar result = new MenuBar();
    result.setLabel(label.getValue(ctx));
    return result;
  }
}
