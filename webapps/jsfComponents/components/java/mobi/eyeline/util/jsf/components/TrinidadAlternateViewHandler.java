package mobi.eyeline.util.jsf.components;

import com.sun.facelets.FaceletViewHandler;

import javax.faces.FacesException;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import java.io.IOException;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class TrinidadAlternateViewHandler extends FaceletViewHandler {

  public TrinidadAlternateViewHandler(javax.faces.application.ViewHandler parent) {
    super(parent);
  }

  private static final String TRINIDAD_VIEW_CACHE_PARAM = "org.apache.myfaces.trinidadinternal.application.VIEW_CACHE";

  private static void replaceState(FacesContext context, String oldKey, String newKey) {
    Map<String, Object> sessionMap = context.getExternalContext().getSessionMap();

    Object newState = sessionMap.remove(TRINIDAD_VIEW_CACHE_PARAM + '.' + newKey);
    sessionMap.put(TRINIDAD_VIEW_CACHE_PARAM + '.' + oldKey, newState);
  }

  private static String getAjaxComponent(FacesContext context) {
    return context.getExternalContext().getRequestParameterMap().get("eyelineComponentUpdate");
  }

  private static boolean isAjaxRequest(FacesContext context) {
    return getAjaxComponent(context) != null;
  }

  @Override
  protected ResponseWriter createResponseWriter(FacesContext context) throws IOException, FacesException {
    if (isAjaxRequest(context)) {
      return new AjaxWriter(super.createResponseWriter(context));
    }else {
      return super.createResponseWriter(context);
    }
  }

  public void renderView(javax.faces.context.FacesContext context, javax.faces.component.UIViewRoot viewToRender) throws java.io.IOException, javax.faces.FacesException {
    String p = getAjaxComponent(context);
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

}
