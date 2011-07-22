package mobi.eyeline.util.jsf.components.sample.controllers;

import java.util.Date;

/**
 * author: Aleksandr Khalitov
 */
public class InputDateController {


  private Date inputDate;

  public String getInputDateString() {
    return inputDate == null ? null : inputDate.toString();
  }

  public Date getInputDate() {
    return inputDate;
  }

  public void setInputDate(Date inputDate) {
    this.inputDate = inputDate;
  }
}
