package mobi.eyeline.util.jsf.components.sample.controllers;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * author: Aleksandr Khalitov
 */
public class InputDateController {


  private Date inputDate;
  private Date inputDate1;
  private Date inputDate2;
  private Date inputDate3;
  private Date inputDate4;

  public String getInputDateString() {
    return inputDate == null ? null : inputDate.toString();
  }

  public Date getInputDate() {
    return inputDate;
  }

  public void setInputDate(Date inputDate) {
    this.inputDate = inputDate;
  }

  public Date getInputDate1() {
    return inputDate1;
  }

  public void setInputDate1(Date inputDate1) {
    this.inputDate1 = inputDate1;
  }

  public Date getInputDate2() {
    return inputDate2;
  }

  public void setInputDate2(Date inputDate2) {
    this.inputDate2 = inputDate2;
  }

  public Date getInputDate3() {
    return inputDate3;
  }

  public void setInputDate3(Date inputDate3) {
    this.inputDate3 = inputDate3;
  }

  public Date getInputDate4() {
    return inputDate4;
  }

  public void setInputDate4(Date inputDate4) {
    if (inputDate4 == null)
      throw new IllegalArgumentException("Invalid date!!!");
    this.inputDate4 = inputDate4;
  }

  public Date getMinDate() {
    try {
      return new SimpleDateFormat("yyyy.MM.dd").parse("2011.07.01");
    } catch (ParseException e) {
      return new Date();
    }
  }

  public Date getMaxDate() {
    try {
      return new SimpleDateFormat("yyyy.MM.dd").parse("2011.09.01");
    } catch (ParseException e) {
      return new Date();
    }
  }
}
