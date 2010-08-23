package ru.novosoft.smsc.web.components.collapsing_group;

import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

/**
 * @author Artem Snopkov
 */
public class CollapsingGroupHandler extends ComponentHandler {

  private final TagAttribute label;
  private final TagAttribute opened;

  public CollapsingGroupHandler(ComponentConfig config) {
    super(config);

    label = getRequiredAttribute("label");
    opened = getAttribute("opened");
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    CollapsingGroup result = new CollapsingGroup();
    result.setLabel(label.getValue(ctx));
    if (opened != null)
      result.setOpened(opened.getBoolean(ctx));
    return result;
  }
  
}
