package ru.novosoft.smsc.web.components.collapsing_group;

import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

/**
 * @author Artem Snopkov
 */
public class CollapsingGroupHandler extends ComponentHandler {

  public CollapsingGroupHandler(ComponentConfig config) {
    super(config);

    System.out.println(this.getAttribute("label"));
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    CollapsingGroup result = new CollapsingGroup();
    result.setLabel(this.getAttribute("label").getValue(ctx));
    TagAttribute opened = this.getAttribute("opened");
    if (opened != null)
      result.setOpened(opened.getBoolean(ctx));
    return result;
  }
  
}
