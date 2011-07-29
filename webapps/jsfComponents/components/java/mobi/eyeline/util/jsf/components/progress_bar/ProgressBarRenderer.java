package mobi.eyeline.util.jsf.components.progress_bar;

import mobi.eyeline.util.jsf.components.HtmlWriter;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * User: artem
 * Date: 29.07.11
 */
public class ProgressBarRenderer extends Renderer {

  public void encodeBegin(FacesContext context, UIComponent component) throws java.io.IOException {
    ProgressBar pb = (ProgressBar)component;

    HtmlWriter w = new HtmlWriter(context.getResponseWriter());

    int percent = pb.getCurrent() * 100 / pb.getMaximum();

    w.a("<div id=\"").a(pb.getId()).a("\" name=\"").a(pb.getId()).a("\" class=\"eyeline_progress_bar_border\">");
    w.a("<div style=\"width:").a(String.valueOf(percent)).a("%\" class=\"eyeline_progress_bar_content\"></div>");
    w.a("</div>");
  }
}
