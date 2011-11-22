package mobi.eyeline.util.jsf.components.menubar;

import mobi.eyeline.util.jsf.components.HtmlWriter;

import javax.faces.component.UIComponent;
import javax.faces.render.Renderer;

/**
 * @author Artem Snopkov
 */
public class MenuBarRenderer extends Renderer {

  private boolean hasParentMenuBar(UIComponent c) {
    return c.getParent() instanceof MenuBar;
  }

  private void prepareItems(UIComponent menuBar) {
    MenuBarItem lastItem = null;
    for (UIComponent c : menuBar.getChildren()) {
      if (c instanceof MenuBarItem)
        lastItem = (MenuBarItem)c;
      else if (c instanceof MenuBarDelimiter && lastItem != null) {
        lastItem.setHasDelimiter(true);
      }
    }
  }

  public void encodeBegin(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws java.io.IOException {
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());

    if (component instanceof MenuBar) {
      prepareItems(component);

      String id = component.getId();
      if (!hasParentMenuBar(component)) {
        w.a("\n<div class=\"menu-bar\" id=\"").a(id).a("\">");
        w.a("\n<a href=\"#\">").a(((MenuBar) component).getLabel()).a("</a>");
        w.a("<ul class=\"sub_menu\">");
      } else {
        if (((MenuBar)component).hasDelimiter())
          w.a("\n<li class=\"menubardelimiter\"><a href=\"#\">").a(((MenuBar) component).getLabel()).a("</a>");
        else
          w.a("\n<li><a href=\"#\">").a(((MenuBar) component).getLabel()).a("</a>");
        w.a("<ul class=\"sub_menu\">");
      }
    } else if (component instanceof MenuBarItem) {
      if (((MenuBarItem)component).hasDelimiter())
        w.a("\n<li class=\"menubardelimiter\">");
      else
        w.a("\n<li>");
    }
  }

  public void encodeEnd(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws java.io.IOException {
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());

    if (component instanceof MenuBar) {
      if (!hasParentMenuBar(component)) {
        w.a("\n</ul></div>");
        w.a("\n<script>$(function(){new Menu($(\"#" + component.getId() +  "\"))});</script>");
      } else {
        w.a("\n</ul></li>");
      }
    } else if (component instanceof MenuBarItem) {
      w.a("\n</li>");
    }
  }
}
