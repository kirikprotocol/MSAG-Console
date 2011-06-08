package mobi.eyeline.util.jsf.components.page_calendar;

import mobi.eyeline.util.jsf.components.AjaxFacesContext;
import mobi.eyeline.util.jsf.components.ResourceUtils;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.io.Writer;
import java.text.SimpleDateFormat;
import java.util.Calendar;

/**
 * @author Artem Snopkov
 */
public class PageCalendarRenderer extends Renderer {


  private void importResources(ResponseWriter writer) throws IOException{
    String baseURL = ResourceUtils.getResourceUrl("");

    writer.startElement("script", null);
    writer.writeAttribute("type", "text/javascript", null);
    writer.writeAttribute("src", baseURL +"js/ajax.js", null);
    writer.endElement("script");

    writer.startElement("script", null);
    writer.writeAttribute("type", "text/javascript", null);
    writer.writeAttribute("src", baseURL +"js/page_calendar.js", null);
    writer.endElement("script");
  }

  private boolean ajax = false;

  public void decode(FacesContext context, UIComponent component) {
    decodeCalendar(context, (PageCalendar) component);
  }

  static void decodeCalendar(FacesContext context, PageCalendar cal) {
    String dateStr = context.getExternalContext().getRequestParameterMap().get(cal.getId() + "_date");
    if (dateStr != null) {
      long date = Long.parseLong(dateStr);
      cal.setDate(date);
    }
  }

  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    ajax = false;
    if (context instanceof AjaxFacesContext) {
      AjaxFacesContext ctx = (AjaxFacesContext) context;
      if (ctx.getAjaxComponentId().equals(component.getId())) {
        ajax = true;
        ctx.setSkipContent(false);
      }
    }

    PageCalendar cal = (PageCalendar) component;
    ResponseWriter w = context.getResponseWriter();

    if (!ajax) {
      importResources(w);
      w.append("<input type=\"hidden\" id=\"" + cal.getId() + "_date\" name=\"" + cal.getId() + "_date\" value=\"" + cal.getDate() + "\"/>");
      w.append("\n<div id=\"" + cal.getId() + "\">");
    }

    Calendar c = Calendar.getInstance();
    c.setTimeInMillis(cal.getDate());
    c.set(Calendar.DAY_OF_WEEK, Calendar.MONDAY);

    Calendar c1 = Calendar.getInstance();
    c1.setTime(c.getTime());

    c1.add(Calendar.DATE, -2);
    long prevWeekDate = c1.getTimeInMillis();
    c1.add(Calendar.DATE, 9);
    long nextWeekDate = c1.getTimeInMillis();

    w.append("\n<table class=\"eyeline_page_calendar_header\"><tr>");
    w.append("\n<td class=\"eyeline_prev\"><a href=\"#\" onclick=\"pageCal" + cal.getId() + ".setDate(" + prevWeekDate + ")\"><div class=\"eyeline_page_calendar_header_prev\">&nbsp;</div></a>");
    w.append("\n<td class=\"eyeline_header\">");
    w.append(c.get(Calendar.YEAR) + "").append("&#160;").append(c.get(Calendar.WEEK_OF_YEAR) + "");
    w.append("\n</td>");
    w.append("\n<td class=\"eyeline_next\"><a href=\"#\" onclick=\"pageCal" + cal.getId() + ".setDate(" + nextWeekDate + ")\"><div class=\"eyeline_page_calendar_header_next\">&nbsp;</div></a>");
    w.append("\n</tr>\n</table>");

    w.append("\n<table class=\"eyeline_page_calendar\">");
    w.append("\n<thead>");

    SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy");

    for (int i = 0; i < 7; i++) {
      w.append("\n<th>").append(sdf.format(c.getTime())).append("</th>");
      c.set(Calendar.DATE, c.get(Calendar.DATE) + 1);
    }

    w.append("\n</thead>\n<tbody>\n<tr>");
  }

  @Override
  public void encodeChildren(FacesContext context, UIComponent component) throws IOException {
    Writer w = context.getResponseWriter();
    for (UIComponent child : component.getChildren()) {
      w.append("\n<td>");
      child.encodeAll(context);
      w.append("\n</td>");
    }
  }

  @Override
  public boolean getRendersChildren() {
    return true;
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    PageCalendar cal = (PageCalendar) component;
    Writer w = context.getResponseWriter();

    w.append("\n</tr>\n</thead>\n</table>");

    if (!ajax) {
      w.append("</div>");
      w.append("\n<script language=\"javascript\" type=\"text/javascript\">");
      w.append("\npageCal" + cal.getId() + "=new PageCalendar('" + cal.getId() + "');");
      w.append("\n</script>");
    }

    if (ajax && (context instanceof AjaxFacesContext))
      ((AjaxFacesContext) context).setSkipContent(true);
  }
}