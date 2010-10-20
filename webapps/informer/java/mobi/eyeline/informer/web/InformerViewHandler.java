package mobi.eyeline.informer.web;

import com.sun.facelets.FaceletViewHandler;
import mobi.eyeline.informer.web.components.AjaxFacesContext;
import org.apache.log4j.Logger;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import java.util.Locale;

/**
 * author: alkhal
 */
public class InformerViewHandler extends FaceletViewHandler {

  private static final Logger logger = Logger.getLogger(InformerViewHandler.class);

  public InformerViewHandler(javax.faces.application.ViewHandler parent) {
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
    Locale l = (Locale)context.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    if(l != null) {
      return l;
    }else {
      return super.calculateLocale(context);
    }
  }
  
}
