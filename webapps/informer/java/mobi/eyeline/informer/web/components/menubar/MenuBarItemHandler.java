package mobi.eyeline.informer.web.components.menubar;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.el.ELException;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class MenuBarItemHandler extends ComponentHandler {


  public MenuBarItemHandler(ComponentConfig config) {
    super(config);
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    return new MenuBarItem();
  }

  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws IOException, FacesException, ELException {
    nextHandler.apply(ctx, c);
  }
}
