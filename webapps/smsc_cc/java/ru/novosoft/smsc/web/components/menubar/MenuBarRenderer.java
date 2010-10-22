package ru.novosoft.smsc.web.components.menubar;

import javax.faces.component.UIComponent;
import javax.faces.render.Renderer;
import java.io.Writer;

/**
 * @author Artem Snopkov
 */
public class MenuBarRenderer extends Renderer {

  public void encodeBegin(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws java.io.IOException {
    Writer w = context.getResponseWriter();

    if (component instanceof MenuBar) {
      String id = component.getId();
      w.append("<div class=\"menubardiv\" id=\"menubar" + id + "\">");
      w.append(((MenuBar) component).getLabel());
      w.append("</div>");

      w.append("<div class=\"menudiv\" id=\"menubarcontent" + id + "\">");
      w.append("<script type=\"text/javascript\">var menubar" + id + "= new MenuBar('menubar" + id + "', 'menubarcontent" + id + "');</script>");
    } else if (component instanceof MenuBarItem) {

      w.append("<div id=\"" + component.getId() + "\" class=\"menuitem\">");
    }
  }

  public void encodeEnd(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws java.io.IOException {
    Writer w = context.getResponseWriter();

    if (component instanceof MenuBar) {
      w.append("</div>");
    } else if (component instanceof MenuBarItem) {

      UIComponent c = component;
      while(c.getParent() != null) {
        c = c.getParent();
        if (c instanceof MenuBar) {
          break;
        }
      }

      w.append("</div>");
      w.append("<script type=\"text/javascript\">menubar" + c.getId() + ".addMenuItem(new MenuItem('" + component.getId() + "'));</script>");
    }
  }
}
