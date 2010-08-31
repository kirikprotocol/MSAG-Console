package ru.novosoft.smsc.web;

import com.sun.facelets.FaceletViewHandler;
import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.web.components.AjaxFacesContext;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import java.security.Principal;
import java.util.Locale;

/**
 * author: alkhal
 */
public class SmscViewHandler extends FaceletViewHandler {

  private static final Logger logger = Logger.getLogger(SmscViewHandler.class);

  public SmscViewHandler(javax.faces.application.ViewHandler parent) {
    super(parent);
  }

  private UIComponent lookupComponent(UIComponent root, String id) {
    if (root.getId() != null && root.getId().equals(id))
      return root;
    for (UIComponent c : root.getChildren()) {
      UIComponent childC = lookupComponent(c, id);
      if (childC != null)
        return childC;
    }
    return null;
  }

  private void deepDecode(FacesContext ctx, UIComponent c) {
    c.decode(ctx);
    for (UIComponent cc : c.getChildren())
      deepDecode(ctx, cc);
  }

  public void renderView(javax.faces.context.FacesContext context, javax.faces.component.UIViewRoot viewToRender) throws java.io.IOException, javax.faces.FacesException {

    String p = context.getExternalContext().getRequestParameterMap().get("eyelineComponentUpdate");
    if (p != null) {
//      buildView(context, viewToRender);
//      final UIComponent c = lookupComponent(viewToRender, p);
//      deepDecode(context, c);
      super.renderView(new AjaxFacesContext(context, p), viewToRender);
    } else {
      super.renderView(context, viewToRender);
    }
  }

  @Override
  public Locale calculateLocale(FacesContext context) {
    Principal p = context.getExternalContext().getUserPrincipal();
    if(p != null) {
      User u = null;
      try {
        u = WebContext.getInstance().getAppliableConfiguration().getUsersSettings().getUser(p.getName());
      } catch (AdminException e) {
        logger.error(e,e);
      }
      if(u != null) {
        return u.getPrefs().getLocale();
      }
    }
    return super.calculateLocale(context);
  }
}
