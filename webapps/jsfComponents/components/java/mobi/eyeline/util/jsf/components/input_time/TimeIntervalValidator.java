package mobi.eyeline.util.jsf.components.input_time;

import mobi.eyeline.util.Time;
import mobi.eyeline.util.jsf.components.MessageUtils;

import javax.faces.application.FacesMessage;
import javax.faces.component.StateHolder;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.validator.Validator;
import javax.faces.validator.ValidatorException;

/**
 * User: artem
 * Date: 27.07.11
 */
public class TimeIntervalValidator implements Validator, StateHolder {

  private static final String MIN_ERROR_ID = "mobi.eyeline.util.jsf.components.input_time.TimeIntervalValidator.MIN";
  private static final String MAX_ERROR_ID = "mobi.eyeline.util.jsf.components.input_time.TimeIntervalValidator.MAX";
  private static final String BOTH_ERROR_ID = "mobi.eyeline.util.jsf.components.input_time.TimeIntervalValidator.BOTH";

  private boolean trans;
  private String minTime;
  private String maxTime;

  public void validate(FacesContext facesContext, UIComponent uiComponent, Object o) throws ValidatorException {
    Time t = (Time)o;
    if (minTime != null) {
      Time minT = new Time(minTime);
      if (t.compareTo(minT) < 0)
        throw new ValidatorException(getErrorMessage(facesContext));
    }
    if (maxTime != null) {
      Time maxT = new Time(maxTime);
      if (t.compareTo(maxT) > 0)
        throw new ValidatorException(getErrorMessage(facesContext));
    }
  }

  private FacesMessage getErrorMessage(FacesContext ctx) {
    if (minTime != null && maxTime != null) {
      return MessageUtils.getErrorMessage(ctx, BOTH_ERROR_ID, minTime, maxTime);
    } else if (maxTime != null ) {
      return MessageUtils.getErrorMessage(ctx, MAX_ERROR_ID, maxTime);
    } else if (minTime != null) {
      return MessageUtils.getErrorMessage(ctx, MIN_ERROR_ID, minTime);
    }
    return new FacesMessage("Invalid time");
  }

  public void setMaxTime(String maxTime) {
    this.maxTime = maxTime;
  }

  public void setMinTime(String minTime) {
    this.minTime = minTime;
  }

  public Object saveState(FacesContext facesContext) {
    Object[] values = new Object[2];
    values[0] = minTime;
    values[1] = maxTime;
    return (values);
  }

  public void restoreState(FacesContext facesContext, Object o) {
    Object[] values = (Object[]) o;
    minTime = (String) values[0];
    maxTime = (String) values[1];
  }

  public boolean isTransient() {
    return trans;
  }

  public void setTransient(boolean b) {
    this.trans = b;
  }
}
