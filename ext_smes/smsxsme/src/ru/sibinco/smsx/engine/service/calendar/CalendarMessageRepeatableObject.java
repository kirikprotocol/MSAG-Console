package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.utils.queue.delayed.DelayedObject;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarMessage;

/**
 * User: artem
 * Date: 11.07.2007
 */

class CalendarMessageRepeatableObject extends DelayedObject {

  private final CalendarMessage msg;

  public CalendarMessageRepeatableObject(CalendarMessage msg) {
    this.msg = msg;
  }

  public CalendarMessage getMsg() {
    return msg;
  }
}
