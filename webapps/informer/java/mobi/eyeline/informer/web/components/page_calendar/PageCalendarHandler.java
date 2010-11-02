package mobi.eyeline.informer.web.components.page_calendar;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.el.ELException;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;
import java.util.Calendar;
import java.util.Date;

/**
 * @author Artem Snopkov
 */
public class PageCalendarHandler extends ComponentHandler {

  private final TagAttribute var;
  private final TagAttribute value;

  public PageCalendarHandler(ComponentConfig config) {
    super(config);

    var = getRequiredAttribute("var");
    value = getRequiredAttribute("value");
  }

   protected UIComponent createComponent(FaceletContext ctx) {
     PageCalendar r = new PageCalendar();
     r.setVar(var.getValue(ctx));
     return r;
   }

  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws IOException, FacesException, ELException {
    PageCalendar cal = (PageCalendar)c;

    Calendar calendar = Calendar.getInstance();
    calendar.setTimeInMillis(cal.getDate());

    calendar.set(Calendar.DAY_OF_WEEK, Calendar.MONDAY);
    Date startDate = calendar.getTime();
    calendar.set(Calendar.DATE, calendar.get(Calendar.DATE) + 6);
    Date endDate = calendar.getTime();

    PageCalendarModel model = (PageCalendarModel)value.getObject(ctx);
    model.updateVisiblePeriod(startDate, endDate);

    calendar.setTimeInMillis(cal.getDate());

    for (int i=2; i<=8; i++) {
      calendar.set(Calendar.DAY_OF_WEEK, i % 7);
      ConstantExpression expr = cal.getVarExpression(i - 2);
      expr.setValue(ctx, model.getValue(calendar.getTime()));
      ctx.getVariableMapper().setVariable(cal.getVar(), expr);
      nextHandler.apply(ctx, c);
    }
  }
}
