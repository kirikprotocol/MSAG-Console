package ru.novosoft.smsc.web.components.buttons;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.io.Writer;

/**
 * @author Artem Snopkov
 */
public class ButtonsRenderer extends Renderer {

  public void decode(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) {
    String selectedButton = context.getExternalContext().getRequestParameterMap().get(component.getId() + "_button");
    if (selectedButton != null && selectedButton.trim().length() > 0)
      ((Buttons)component).setSelectedButton(selectedButton);
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {

    

    Writer w = context.getResponseWriter();
    String contextPath = context.getExternalContext().getRequestContextPath();

    String buttonElementId = component.getId() + "_button";

    w.append("\n<input type=\"hidden\" id=\"" + buttonElementId + "\" name=\"" + buttonElementId + "\" value=\"\"/>");
    w.append("\n<script language=\"javascript\" type=\"text/javascript\">");
    w.append("\nfunction clickButton" + component.getId() + "(value) {");
    w.append("\n  document.getElementById('" + buttonElementId + "').value=value;");
    w.append("\n  for (var k=0; k<document.forms.length; k++) {");
    w.append("\n    if (document.forms[k].elements['" + buttonElementId + "'] != null)");
    w.append("\n      return document.forms[k].submit();");
    w.append("\n  }");
    w.append("\n}");
    w.append("\n</script>");

    w.append("\n<div class=\"page_menu\">");
    w.append("\n<table class=\"page_menu_background\" cellspacing=\"0px\" cellpadding=\"0px\">");
    w.append("\n<tr>");
    w.append("\n<td background=\"" + contextPath + "/images/smsc_21.jpg\">&nbsp;</td>");
    w.append("\n</tr>");
    w.append("\n</table>");
    w.append("\n<table class=\"page_menu\" cellpadding=\"0px\" cellspacing=\"0px\">");
    w.append("\n<tr>");
    w.append("\n<td width=\"37px\">&nbsp;</td>");

    Buttons b = (Buttons)component;

    int i=0;
    for (Object o : b.getElements()) {
      if (o instanceof Space) {
        w.append("<td>&nbsp;</td>");
        i=0;
      } else if (o instanceof Button) {
        if (i > 0)
          w.append("<td width=\"1px\">|</td>");
        Button button = (Button)o;
        w.append("\n<td width=\"1px\">");
        w.append("\n<a title=\"" + button.getTitle() + "\" onclick=\"return clickButton" + component.getId() + "('" + button.getAction() + "');\">");
        w.append(button.getTitle());
        w.append("\n</a></td>");
        i++;
      }
    }    
    w.append("\n<td width=\"37px\">&nbsp;</td>");
    w.append("\n</tr>");
    w.append("\n</table>");
    w.append("\n</div>");
  }
}
