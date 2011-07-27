package mobi.eyeline.util.jsf.components.sample.controllers;

import mobi.eyeline.util.Time;

import javax.faces.FacesException;
import javax.faces.context.FacesContext;
import javax.faces.convert.ConverterException;

/**
 * author: Aleksandr Khalitov
 */
public class InputTimeController {

  private Time time1;
  private Time time2 = new Time(12,49,0);
  private Time time3;
  private Time time4;
  private Time time5;
  private Time time6;


  public Time getTime1() {
    return time1;
  }

  public void setTime1(Time time1) {
    this.time1 = time1;
  }

  public Time getTime2() {
    return time2;
  }

  public void setTime2(Time time2) {
    this.time2 = time2;
  }

  public Time getTime3() {
    return time3;
  }

  public void setTime3(Time time3) {
    if (time3 != null && time3.getHour() > 12)
      throw new IllegalArgumentException("Time3 too big!");
    this.time3 = time3;
  }

  public Time getTime4() {
    return time4;
  }

  public void setTime4(Time time4) {
    this.time4 = time4;
  }

  public Time getTime5() {
    return time5;
  }

  public void setTime5(Time time5) {
    this.time5 = time5;
  }

  public Time getTime6() {
    return time6;
  }

  public void setTime6(Time time6) {
    this.time6 = time6;
  }

  public String submit() {
    String bundleName = FacesContext.getCurrentInstance().getApplication().getMessageBundle();
    System.out.println("Bundle: " + bundleName);
    return null;
  }
}
