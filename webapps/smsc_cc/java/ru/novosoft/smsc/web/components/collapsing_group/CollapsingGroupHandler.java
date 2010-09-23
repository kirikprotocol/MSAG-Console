package ru.novosoft.smsc.web.components.collapsing_group;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.el.ELException;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class CollapsingGroupHandler extends ComponentHandler {

  private final TagAttribute label;
  private final TagAttribute opened;
  private final TagAttribute rendered;

  public CollapsingGroupHandler(ComponentConfig config) {
    super(config);

    label = getRequiredAttribute("label");
    opened = getAttribute("opened");
    rendered = getAttribute("rendered");
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    CollapsingGroup result = new CollapsingGroup();
    result.setLabel(label.getValue(ctx));
    if (opened != null)
      result.setOpened(opened.getBoolean(ctx));
    return result;
  }

  @Override
  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws IOException, FacesException, ELException {
    if (rendered != null && !rendered.getBoolean(ctx))
      return;

    CollapsingGroup g = (CollapsingGroup)c;

    g.setLabel(label.getValue(ctx));
    if (opened != null)
      g.setOpened(opened.getBoolean(ctx));

    nextHandler.apply(ctx, c);
  }
}
