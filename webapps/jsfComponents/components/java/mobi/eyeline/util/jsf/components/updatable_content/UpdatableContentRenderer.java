package mobi.eyeline.util.jsf.components.updatable_content;

import mobi.eyeline.util.jsf.components.AjaxFacesContext;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;

/**
 * @author Aleksandr Khalitov
 */
public class UpdatableContentRenderer extends Renderer {

  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    if (context instanceof AjaxFacesContext) {
      AjaxFacesContext ctx = (AjaxFacesContext) context;
      if (ctx.getAjaxComponentId().equals(component.getId())) {
        ((AjaxFacesContext) context).setSkipContent(false);
        return;
      }
    }
    UpdatableContent pb = (UpdatableContent) component;
    ResponseWriter w = context.getResponseWriter();

    w.append("\n<div id=\"").append(pb.getId()).append("\">");
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    UpdatableContent pb = (UpdatableContent) component;
    ResponseWriter w = context.getResponseWriter();

    if (pb.isEnabled() && pb.getUpdatePeriod() > 0) {
      ((HttpServletResponse)context.getExternalContext().getResponse()).addHeader("nextUpdateTimeout", String.valueOf(pb.getUpdatePeriod() * 1000));
    }

    if (context instanceof AjaxFacesContext) {
      AjaxFacesContext ctx = (AjaxFacesContext) context;
      if (ctx.getAjaxComponentId().equals(component.getId())) {
        ((AjaxFacesContext) context).setSkipContent(true);
        return;
      }
    }

    w.append("\n</div>");
    w.append("\n<script language=\"javascript\" type=\"text/javascript\">");
    w.append("\n  initUpdatableContent('" + pb.getId() + "'," + (pb.getUpdatePeriod()*1000) + ");");
    w.append("\n</script>");


  }

}
