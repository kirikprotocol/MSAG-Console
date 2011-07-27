package mobi.eyeline.util.jsf.components.message;

import mobi.eyeline.util.jsf.components.HtmlWriter;
import mobi.eyeline.util.jsf.components.MessageUtils;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.io.IOException;

/**
 * User: artem
 * Date: 27.07.11
 */
public class MessageRenderer extends Renderer {

  private static final String ERROR_TEXT = "mobi.eyeline.util.jsf.components.message.Message.ERROR";
  private static final String WARN_TEXT = "mobi.eyeline.util.jsf.components.message.Message.WARN";
  private static final String INFO_TEXT = "mobi.eyeline.util.jsf.components.message.Message.INFO";

  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    Message m = (Message) component;

    String level = m.getLevel();
    if (level == null)
      level = "info";

    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    w.a("<table class=\"eyeline_message\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\" summary=\"\">");
    w.a("<tbody><tr><td>");
    if (level.equals("error")) {
      w.a("<div class=\"header_error\">").a(MessageUtils.getMessageString(context, ERROR_TEXT)).a("</div>");
    } else if (level.equals("warn")) {
      w.a("<div class=\"header_warn\">").a(MessageUtils.getMessageString(context, WARN_TEXT)).a("</div>");
    } else {
      w.a("<div class=\"header_info\">").a(MessageUtils.getMessageString(context, INFO_TEXT)).a("</div>");
    }
    w.a("<div class=\"body\">");
  }

  @Override
  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    w.a("</div>");
    w.a("</td></tr></tbody>");
    w.a("</table>");
  }
}
