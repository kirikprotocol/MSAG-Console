package mobi.eyeline.util.jsf.components.input_date;

import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.webapp.UIComponentELTag;

/**
 * User: artem
 * Date: 01.08.11
 */
public class InputDateTag extends UIComponentELTag {

  private ValueExpression minDate;
  private ValueExpression maxDate;
  private ValueExpression numberOfMonths;
  private ValueExpression inputTime;

  public void setInputTime(ValueExpression inputTime) {
    this.inputTime = inputTime;
  }

  public void setMaxDate(ValueExpression maxDate) {
    this.maxDate = maxDate;
  }

  public void setMinDate(ValueExpression minDate) {
    this.minDate = minDate;
  }

  public void setNumberOfMonths(ValueExpression numberOfMonths) {
    this.numberOfMonths = numberOfMonths;
  }

  @Override
  public void release() {
    super.release();
    minDate = null;
    maxDate = null;
    numberOfMonths = null;
    inputTime = null;
  }

  @Override
  protected void setProperties(UIComponent component) {
    super.setProperties(component);

    if (minDate != null)
      component.setValueExpression("minDate", minDate);

    if (maxDate != null)
      component.setValueExpression("maxDate", maxDate);

    if (numberOfMonths != null)
      component.setValueExpression("numberOfMonths", numberOfMonths);

    if (inputTime != null)
      component.setValueExpression("inputTime", inputTime);
  }

  @Override
  public String getComponentType() {
    return "mobi.eyeline.util.jsf.components.input_date";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.input_date";
  }
}
