package mobi.eyeline.informer.web;

import com.sun.facelets.FaceletViewHandler;
import mobi.eyeline.informer.web.components.AjaxFacesContext;

import javax.faces.context.FacesContext;
import java.util.Locale;

/**
 * author: alkhal
 */
public class InformerViewHandler extends FaceletViewHandler {

  public InformerViewHandler(javax.faces.application.ViewHandler parent) {
    super(parent);
  }

  public void renderView(javax.faces.context.FacesContext context, javax.faces.component.UIViewRoot viewToRender) throws java.io.IOException, javax.faces.FacesException {

    String p = context.getExternalContext().getRequestParameterMap().get("eyelineComponentUpdate");
    if (p != null) {
      super.renderView(new AjaxFacesContext(context, p), viewToRender);
    } else {
      super.renderView(context, viewToRender);
    }
  }

  @Override
  public Locale calculateLocale(FacesContext context) {
    Locale l = (Locale) context.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    if (l != null) {
      return l;
    } else {
      return super.calculateLocale(context);
    }
  }

}
