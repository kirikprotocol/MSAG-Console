package mobi.eyeline.util.jsf.components.updatable_content;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.faces.component.UIComponent;

/**
 * @author Aleksandr Khalitov
 */
public class UpdatableContentHandler extends ComponentHandler {

  private final TagAttribute updatePeriod;
  private final TagAttribute enabled;

  public UpdatableContentHandler(ComponentConfig config) {
    super(config);
    updatePeriod = getRequiredAttribute("updatePeriod");
    enabled = getAttribute("enabled");
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    return new UpdatableContent();
  }

  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {
    UpdatableContent updatableContent = (UpdatableContent) c;
    updatableContent.setUpdatePeriod(updatePeriod.getInt(ctx));

    if (enabled != null)
      updatableContent.setEnabled(enabled.getBoolean(ctx));

    nextHandler.apply(ctx, c);
  }
}
