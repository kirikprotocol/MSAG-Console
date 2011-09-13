package mobi.eyeline.util.jsf.components.collapsing_group;

import mobi.eyeline.util.jsf.components.HtmlWriter;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class CollapsingGroupRenderer extends Renderer {

  public void decode(FacesContext context, UIComponent component) {
    CollapsingGroup cg = (CollapsingGroup) component;
    String opened = context.getExternalContext().getRequestParameterMap().get(cg.getId() + "Status");
    cg.setOpened((opened != null && opened.equals("0")));
  }

  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {

    CollapsingGroup cg = (CollapsingGroup) component;
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());

    String openedStr =cg.isOpened() ? "0" : "1";

    w.a("<input type=\"hidden\" id=\"").a(cg.getId()).a("Status\" name=\"").a(cg.getId()).a("Status\" value=\"").a(openedStr).a("\"/>");
    w.a("<div id=\"").a(cg.getId()).a("Header\" name=\"").a(cg.getId()).a("Header\" class=\"eyeline_collapsingGroupHeader").a(openedStr).a("\">");
    UIComponent headerFacet = cg.getFacet("header");
    if (headerFacet != null) {
      headerFacet.encodeBegin(context);
      headerFacet.encodeEnd(context);
    } else {
      w.a(cg.getLabel());
    }
    w.a("</div>");
    w.a("<div id=\"").a(cg.getId()).a("Body\" name=\"").a(cg.getId()).a("Body\" class=\"eyeline_collapsingGroupBody").a(openedStr).a("\">");
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    CollapsingGroup cg = (CollapsingGroup) component;
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    w.a("</div>");
    w.a("<script type=\"text/javascript\">$(function() {initCollapsingGroup(\"").a(cg.getId()).a("\");});</script>");
  }
}
