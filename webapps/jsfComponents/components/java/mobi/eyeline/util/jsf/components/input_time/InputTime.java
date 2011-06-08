package mobi.eyeline.util.jsf.components.input_time;

import mobi.eyeline.util.Time;
import mobi.eyeline.util.jsf.components.EyelineComponent;

import javax.el.ValueExpression;
import javax.faces.context.FacesContext;

/**
 * @author Aleksandr Khalitov
 */
public class InputTime extends EyelineComponent {

  private String errorMessage;
  private Time value;
  private ValueExpression valueExpression;

  public Time getValue() {
    return value;
  }

  public void setValue(Time value) {
    this.value = value;
  }

  public void setValueExpression(ValueExpression valueExpression) {
    this.valueExpression = valueExpression;
  }

  public void processUpdates(javax.faces.context.FacesContext context) {
    if (valueExpression != null)
      valueExpression.setValue(context.getELContext(), value);
    super.processUpdates(context);
  }

  public String getErrorMessage() {
    return errorMessage;
  }

  public void setErrorMessage(String errorMessage) {
    this.errorMessage = errorMessage;
  }

  public void processDecodes(javax.faces.context.FacesContext context) {
    InputTimeRenderer.decodeTime(context, this);
    super.processDecodes(context);
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[4];
    values[0] = super.saveState(context);
    values[1] = value;
    values[2] = valueExpression;
    values[3] = errorMessage;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    value = (Time) values[1];
    valueExpression = (ValueExpression)values[2];
    errorMessage = (String)values[3];
  }

}