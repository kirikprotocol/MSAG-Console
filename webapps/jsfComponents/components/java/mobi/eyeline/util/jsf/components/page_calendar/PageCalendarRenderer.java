package mobi.eyeline.util.jsf.components.page_calendar;

import mobi.eyeline.util.jsf.components.AjaxFacesContext;
import mobi.eyeline.util.jsf.components.HtmlWriter;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;

/**
 * @author Artem Snopkov
 */
public class PageCalendarRenderer extends Renderer {


  private boolean ajax = false;

  public void decode(FacesContext context, UIComponent component) {
    PageCalendar cal = (PageCalendar) component;
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
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());

    if (!ajax) {
      w.a("<input type=\"hidden\" id=\"" + cal.getId() + "_date\" name=\"" + cal.getId() + "_date\" value=\"" + cal.getDate() + "\"/>");
      w.a("\n<div id=\"" + cal.getId() + "\">");
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

    w.a("\n<table class=\"eyeline_page_calendar_header\"><tr>");
    w.a("\n<td class=\"eyeline_prev\"><a href=\"#\" onclick=\"pageCal" + cal.getId() + ".setDate(" + prevWeekDate + ")\"><div class=\"eyeline_page_calendar_header_prev\">&nbsp;</div></a>");
    w.a("\n<td class=\"eyeline_header\">");
    w.a(c.get(Calendar.YEAR) + "").a("&#160;").a(c.get(Calendar.WEEK_OF_YEAR) + "");
    w.a("\n</td>");
    w.a("\n<td class=\"eyeline_next\"><a href=\"#\" onclick=\"pageCal" + cal.getId() + ".setDate(" + nextWeekDate + ")\"><div class=\"eyeline_page_calendar_header_next\">&nbsp;</div></a>");
    w.a("\n</tr>\n</table>");

    w.a("\n<table class=\"eyeline_page_calendar\">");
    w.a("\n<thead>");

    SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy");

    for (int i = 0; i < 7; i++) {
      w.a("\n<th>").a(sdf.format(c.getTime())).a("</th>");
      c.set(Calendar.DATE, c.get(Calendar.DATE) + 1);
    }

    w.a("\n</thead>\n<tbody>\n<tr>");
  }

  @Override
  public void encodeChildren(FacesContext context, UIComponent component) throws IOException {
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    for (UIComponent child : component.getChildren()) {
      w.a("\n<td>");
      child.encodeAll(context);
      w.a("\n</td>");
    }
  }

  @Override
  public boolean getRendersChildren() {
    return true;
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    PageCalendar cal = (PageCalendar) component;
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());

    w.a("\n</tr>\n</thead>\n</table>");

    if (!ajax) {
      w.a("</div>");
      w.a("\n<script language=\"javascript\" type=\"text/javascript\">");
      w.a("\npageCal" + cal.getId() + "=new PageCalendar('" + cal.getId() + "');");
      w.a("\n</script>");
    }

    if (ajax && (context instanceof AjaxFacesContext))
      ((AjaxFacesContext) context).setSkipContent(true);
  }
}