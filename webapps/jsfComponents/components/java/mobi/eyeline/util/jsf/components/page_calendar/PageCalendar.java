package mobi.eyeline.util.jsf.components.page_calendar;

import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * @author Artem Snopkov
 */
public class PageCalendar extends UIPanel {

  private String var;
  private long date;
  private ConstantExpression[] varExpressions;
  private final Renderer renderer = new PageCalendarRenderer();

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

  ConstantExpression getVarExpression(int index) {
    return varExpressions[index];
  }

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return renderer;
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
