package mobi.eyeline.util.jsf.components.tabs;


import mobi.eyeline.util.jsf.components.HtmlWriter;

import javax.faces.component.UIComponent;
import javax.faces.render.Renderer;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;

/**
 * User: artem
 * Date: 21.11.11
 */
public class TabsRenderer extends Renderer {

  private Collection<Tab> getTabs(UIComponent parent) {
    Collection<Tab> res = new ArrayList<Tab>();
    for (UIComponent c : parent.getChildren()) {
      if (c instanceof Tab)
        res.add((Tab) c);
    }
    return res;
  }

  public void encodeBegin(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws java.io.IOException {

    HtmlWriter w = new HtmlWriter(context.getResponseWriter());

    if (component instanceof Tab) {
      w.a("\n<div id=\"").a(component.getId()).a("\">");
    } else if (component instanceof Tabs) {
      w.a("\n<div id=\"" + component.getId() + "\">");
      w.a("\n<ul>");
      Collection<Tab> tabs = getTabs(component);
      for (Tab tab : tabs) {
        w.a("<li><a href=\"#").a(tab.getId()).a("\">").a(tab.getLabel()).a("</a></li>");
      }
      w.a("</ul>");
    }
  }

  public void encodeEnd(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws java.io.IOException {
    Writer w = context.getResponseWriter();
    if (component instanceof Tab) {
      w.append("\n</div>");
    } else if (component instanceof Tabs) {
      w.append("\n</div>");
      w.append("\n<script>");
      w.append("\n$(function(){$(\"#").append(component.getId()).append("\").tabs();});");
      w.append("\n</script>");
    }
  }

}
