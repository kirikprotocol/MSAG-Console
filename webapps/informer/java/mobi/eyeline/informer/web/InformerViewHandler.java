package mobi.eyeline.informer.web;

import com.sun.facelets.FaceletViewHandler;
import mobi.eyeline.informer.web.components.AjaxFacesContext;

import javax.faces.context.FacesContext;
import java.util.Locale;
import java.util.Map;

/**
 * author: alkhal
 */
public class InformerViewHandler extends FaceletViewHandler {

  public InformerViewHandler(javax.faces.application.ViewHandler parent) {
    super(parent);
  }

  private static final String VIEW_CACHE_PARAM = "org.apache.myfaces.trinidadinternal.application.VIEW_CACHE";

  private static void replaceState(FacesContext context, String oldKey, String newKey) {
    Map<String, Object> sessionMap = context.getExternalContext().getSessionMap();

    Object newState = sessionMap.remove(VIEW_CACHE_PARAM + '.' + newKey);
    sessionMap.put(VIEW_CACHE_PARAM + '.' + oldKey, newState);
  }

  public void renderView(javax.faces.context.FacesContext context, javax.faces.component.UIViewRoot viewToRender) throws java.io.IOException, javax.faces.FacesException {

    String p = context.getExternalContext().getRequestParameterMap().get("eyelineComponentUpdate");
    if (p != null) {
      super.renderView(new AjaxFacesContext(context, p), viewToRender);

      Object[] oldState = (Object[])context.getRenderKit().getResponseStateManager().getState(context, viewToRender.getViewId());
      String oldStateStr = oldState.length > 0 ? (String)oldState[0] : null;

      Object[] newState = (Object[]) context.getApplication().getStateManager().saveView(context);
      String newStateStr = newState.length > 0 ? (String) newState[0] : null;

      replaceState(context, oldStateStr, newStateStr);

    } else {
      super.renderView(context, viewToRender);
    }
  }

  @Override
  public Locale calculateLocale(FacesContext context) {
    Locale l = (Locale) context.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    if (l == null)
      return super.calculateLocale(context);
    return l;
  }

}
