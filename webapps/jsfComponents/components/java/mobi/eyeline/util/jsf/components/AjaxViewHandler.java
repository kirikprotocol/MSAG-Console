package mobi.eyeline.util.jsf.components;

import com.sun.facelets.FaceletViewHandler;

/**
 * @author Aleksandr Khalitov
 */
public class AjaxViewHandler extends FaceletViewHandler {

  public AjaxViewHandler(javax.faces.application.ViewHandler parent) {
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

}
