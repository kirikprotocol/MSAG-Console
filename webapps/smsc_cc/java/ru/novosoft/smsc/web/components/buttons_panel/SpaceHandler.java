package ru.novosoft.smsc.web.components.buttons_panel;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.faces.component.UIComponent;

/**
 * @author Artem Snopkov
 */
public class SpaceHandler extends ComponentHandler {
  public SpaceHandler(ComponentConfig config) {
    super(config);
  }

  @Override
  protected UIComponent createComponent(FaceletContext ctx) {
    return new Space();
  }
}
