package mobi.eyeline.util.jsf.components.input_date;

import mobi.eyeline.util.jsf.components.base.UIInputImpl;

import javax.el.ValueExpression;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.util.Date;

/**
 * User: artem
 * Date: 01.08.11
 */
public class InputDate extends UIInputImpl {

  private Date minDate;
  private Date maxDate;
  private Integer numberOfMonths = 1;
  private Boolean inputTime = false;

  public Boolean isInputTime() {
    ValueExpression exp = getValueExpression("inputTime");
    if (exp == null)
      return inputTime;
    else
      return (Boolean) exp.getValue(getFacesContext().getELContext());
  }

  public void setInputTime(Boolean inputTime) {
    this.inputTime = inputTime;
  }

  public Date getMaxDate() {
    ValueExpression exp = getValueExpression("maxDate");
    if (exp == null)
      return maxDate;
    else
      return (Date) exp.getValue(getFacesContext().getELContext());
  }

  public void setMaxDate(Date maxDate) {
    this.maxDate = maxDate;
  }

  public Date getMinDate() {
    ValueExpression exp = getValueExpression("minDate");
    if (exp == null)
      return minDate;
    else
      return (Date) exp.getValue(getFacesContext().getELContext());
  }

  public void setMinDate(Date minDate) {
    this.minDate = minDate;
  }

  public Integer getNumberOfMonths() {
    ValueExpression exp = getValueExpression("numberOfMonths");
    if (exp == null)
      return numberOfMonths;
    else
      return (Integer) exp.getValue(getFacesContext().getELContext());
  }

  public void setNumberOfMonths(Integer numberOfMonths) {
    this.numberOfMonths = numberOfMonths;
  }

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new InputDateRenderer();
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[5];
    values[0] = super.saveState(context);
    values[1] = minDate;
    values[2] = maxDate;
    values[3] = inputTime;
    values[4] = numberOfMonths;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    minDate        = (Date) values[1];
    maxDate        = (Date) values[2];
    inputTime      = (Boolean) values[3];
    numberOfMonths = (Integer) values[4];
  }

}
