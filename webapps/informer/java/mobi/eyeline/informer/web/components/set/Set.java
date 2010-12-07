package mobi.eyeline.informer.web.components.set;

import mobi.eyeline.informer.web.components.EyelineComponent;

import javax.el.ELContext;
import javax.el.ValueExpression;
import javax.faces.context.FacesContext;
import java.util.ArrayList;

/**
 * @author Artem Snopkov
 */
public class Set extends EyelineComponent {

  private String var;
  private ConstantExpression value;
  private ValueExpression expr;

  public String getVar() {
    return var;
  }

  public void setVar(String var) {
    this.var = var;
  }

  public ConstantExpression getValue() {
    return value;
  }

  public void setValue(ValueExpression value, ELContext ctx) {
    if (this.value == null)
      this.value = new ConstantExpression(value.getValue(ctx));
    else
      this.value.setValue(null, value.getValue(ctx));
    this.expr = value;
  }

  public void processUpdates(javax.faces.context.FacesContext context) {
    this.value.setValue(null, expr.getValue(context.getELContext()));
    super.processUpdates(context);
  }

   public Object saveState(FacesContext context) {
    Object[] values = new Object[4];
    values[0] = super.saveState(context);
    values[1] = var;
    values[2] = value;
     values[3] = expr;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    var = (String) values[1];
    value = (ConstantExpression)values[2];
    expr = (ValueExpression)values[3];
  }
}
