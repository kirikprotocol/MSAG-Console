package mobi.eyeline.informer.web.components.updatable_content;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.faces.component.UIComponent;

/**
 * @author Aleksandr Khalitov
 */
public class UpdatableContentHandler extends ComponentHandler{

  private final TagAttribute updatePeriod;

  public UpdatableContentHandler(ComponentConfig config) {
    super(config);
    updatePeriod = getRequiredAttribute("updatePeriod");
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    UpdatableContent updatableContent = new UpdatableContent();
    return updatableContent;
  }

  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {
    UpdatableContent updatableContent = (UpdatableContent)c;
    updatableContent.setUpdatePeriod(updatePeriod.getInt(ctx));

    nextHandler.apply(ctx, c);
  }
}
