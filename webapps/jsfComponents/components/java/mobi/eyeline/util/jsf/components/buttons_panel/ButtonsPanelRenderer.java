package mobi.eyeline.util.jsf.components.buttons_panel;

import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import java.io.Writer;

/**
 * @author Artem Snopkov
 */
public class ButtonsPanelRenderer extends Renderer {

  private int button = 0;


  public void encodeBegin(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws java.io.IOException {

    ResponseWriter w = context.getResponseWriter();

    if (component instanceof ButtonsPannel) {
      button = 0;
      w.append("\n<div class=\"eyeline_buttons\">");
      w.append("\n<table class=\"eyeline_buttons_background\" cellspacing=\"0px\" cellpadding=\"0px\">");
      w.append("\n<tr>");

      w.append("\n<td>&nbsp;</td>");
      w.append("\n</tr>");
      w.append("\n</table>");
      w.append("\n<table class=\"eyeline_buttons\" cellpadding=\"0px\" cellspacing=\"0px\">");
      w.append("\n<tr>");
      w.append("\n<td width=\"37px\">&nbsp;</td>");

    } else if (component instanceof Space) {
      w.append("<td>&nbsp;</td>");
      button = 0;
    } else {
      if (button++ > 0)
        w.append("<td width=\"1px\" style=\"padding-left:4px; padding-right:4px\">|</td>");
      w.append("\n<td width=\"1px\">");
    }
  }

  public void encodeEnd(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws java.io.IOException {
    Writer w = context.getResponseWriter();

    if (component instanceof ButtonsPannel) {
      w.append("\n<td width=\"37px\">&nbsp;</td>");
      w.append("\n</tr>");
      w.append("\n</table>");
      w.append("\n</div>");
    } else if (component instanceof Button) {
      w.append("\n</td>");
    }
  }

}
