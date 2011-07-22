package mobi.eyeline.util.jsf.components.sample.controllers;

import mobi.eyeline.util.Time;

/**
 * author: Aleksandr Khalitov
 */
public class InputTimeController {

  private Time time;


  public Time getTime() {
    return time;
  }

  public void setTime(Time time) {
    this.time = time;
  }

  public String getTimeString() {
    return time == null ? null : time.toString();
  }

}
