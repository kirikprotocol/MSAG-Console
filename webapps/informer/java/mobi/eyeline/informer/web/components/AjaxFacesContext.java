package mobi.eyeline.informer.web.components;

import org.apache.myfaces.context.FacesContextWrapper;

import javax.faces.context.FacesContext;

/**
 * @author Artem Snopkov
 */
public class AjaxFacesContext extends FacesContextWrapper {

  private final String ajaxComponentId;

  public AjaxFacesContext(FacesContext facesContext, String ajaxComponentId) {
    super(facesContext);
    this.ajaxComponentId = ajaxComponentId;
  }

  public void setSkipContent(boolean skipContent) {
    ((AjaxWriter)getResponseWriter()).setSkipContent(skipContent);
  }

  public String getAjaxComponentId() {
    return ajaxComponentId;
  }

}
