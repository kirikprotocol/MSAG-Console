package mobi.eyeline.util.jsf.components.input_time;

import javax.el.ValueExpression;
import javax.faces.context.FacesContext;
import javax.faces.validator.Validator;
import javax.faces.webapp.ValidatorELTag;
import javax.servlet.jsp.JspException;

/**
 * User: artem
 * Date: 27.07.11
 */
public class TimeIntervalValidatorTag extends ValidatorELTag {

  private ValueExpression minTime;
  private ValueExpression maxTime;

  public void setMaxTime(ValueExpression maxTime) {
    this.maxTime = maxTime;
  }

  public void setMinTime(ValueExpression minTime) {
    this.minTime = minTime;
  }

  private static Object getExprValue(ValueExpression exp, FacesContext ctx) {
    if (exp == null)
      return null;
    if (!exp.isLiteralText())
      return exp.getValue(ctx.getELContext());
    else
      return exp.getExpressionString();
  }

  @Override
  protected Validator createValidator() throws JspException {
    TimeIntervalValidator res = new TimeIntervalValidator();
    res.setMinTime((String)getExprValue(minTime, FacesContext.getCurrentInstance()));
    res.setMaxTime((String)getExprValue(minTime, FacesContext.getCurrentInstance()));
    return res;
  }
}
