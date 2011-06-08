package mobi.eyeline.util.jsf.components.page_calendar;

import mobi.eyeline.util.jsf.components.EyelineComponent;

import javax.faces.context.FacesContext;

/**
 * @author Artem Snopkov
 */
public class PageCalendar extends EyelineComponent {

  private String var;
  private long date;
  private ConstantExpression[] varExpressions;

  public PageCalendar() {
    date = System.currentTimeMillis();
    varExpressions = new ConstantExpression[7];
    for (int i = 0; i < varExpressions.length; i++)
      varExpressions[i] = new ConstantExpression(0);
  }

  public String getVar() {
    return var;
  }

  public void setVar(String var) {
    this.var = var;
  }

  public long getDate() {
    return date;
  }

  public void setDate(long date) {
    this.date = date;
  }

  public ConstantExpression getVarExpression(int index) {
    return varExpressions[index];
  }

  public void processDecodes(javax.faces.context.FacesContext context) {
    PageCalendarRenderer.decodeCalendar(context, this);
    super.processDecodes(context);
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[4];
    values[0] = super.saveState(context);
    values[1] = var;
    values[2] = date;
    values[3] = varExpressions;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    var = (String) values[1];
    date = (Long) values[2];
    varExpressions = (ConstantExpression[]) values[3];
  }
}
