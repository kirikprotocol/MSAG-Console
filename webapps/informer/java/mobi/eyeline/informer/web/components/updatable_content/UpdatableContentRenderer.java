package mobi.eyeline.informer.web.components.updatable_content;

import mobi.eyeline.informer.web.components.AjaxFacesContext;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import java.io.IOException;

/**
 * @author Aleksandr Khalitov
 */
public class UpdatableContentRenderer extends Renderer {

//  @Override
//  public boolean getRendersChildren() {
//    return true;
//  }

  protected void encodeAjaxPart(FacesContext context, ResponseWriter w, UIComponent component) throws IOException {
//    w.append("dsasda");
    try{
      if(context instanceof AjaxFacesContext) {
      }
      encodeChildren(context, component);
    }finally {
      if(context instanceof AjaxFacesContext) {
        ((AjaxFacesContext)context).setSkipContent(true);
      }
    }
  }


  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    if(context instanceof AjaxFacesContext) {
      AjaxFacesContext ctx = (AjaxFacesContext) context;
      if (ctx.getAjaxComponentId().equals(component.getId())) {
        ((AjaxFacesContext)context).setSkipContent(false);
        return;
      }
    }
    UpdatableContent pb = (UpdatableContent)component;
    ResponseWriter w = context.getResponseWriter();
    w.append("\n<div id=\"").append(pb.getId()).append("\">");
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    if(context instanceof AjaxFacesContext) {
      AjaxFacesContext ctx = (AjaxFacesContext) context;
      if (ctx.getAjaxComponentId().equals(component.getId())) {
        ((AjaxFacesContext)context).setSkipContent(true);
        return;
      }
    }
    UpdatableContent pb = (UpdatableContent)component;
    ResponseWriter w = context.getResponseWriter();
    w.append("\n</div>");

    w.append("\n<script language=\"javascript\" type=\"text/javascript\">");

    w.append("\nvar updateContent" + pb.getId() + "=new UpdateContent('" + pb.getId() + "');");

    w.append("\nfunction autoUpdate" + pb.getId() + "(){");
    w.append("\n  updateContent" + pb.getId() + ".update();");
    w.append("\n  window.setTimeout(autoUpdate" + pb.getId() + "," + pb.getUpdatePeriod()*1000 + ");");
    w.append("\n};");
    w.append("\n  window.setTimeout(autoUpdate" + pb.getId() + "," + pb.getUpdatePeriod()*1000 + ");");
    w.append("\n</script>");

  }

}
