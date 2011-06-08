package mobi.eyeline.util.jsf.components.collapsing_group;

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

  @Override
  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws IOException, FacesException, ELException {
    CollapsingGroup g = (CollapsingGroup) c;

    g.setLabel(label.getValue(ctx));
    if (opened != null)
      g.setOpened(opened.getBoolean(ctx));

    nextHandler.apply(ctx, c);
  }
}
