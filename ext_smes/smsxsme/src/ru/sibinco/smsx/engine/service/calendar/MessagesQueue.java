package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.utils.queue.delayed.DelayedQueue;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarMessage;

import java.util.Date;

/**
 * User: artem
 * Date: 11.07.2007
 */

class MessagesQueue extends DelayedQueue {

  private final Date maxDate = new Date();

  public void setMaxDate(Date maxDate) {
    synchronized(this.maxDate) {
      this.maxDate.setTime(maxDate.getTime());
    }    
  }

  public boolean add(final CalendarMessage obj) {
    synchronized(maxDate) {
      if (obj.getSendDate().after(maxDate))
        return false;
    }

    final CalendarMessageRepeatableObject o = new CalendarMessageRepeatableObject(obj);
    o.setNextTime(obj.getSendDate().getTime());
    addObject(o);
    return true;
  }

  public CalendarMessage getNext() {
    CalendarMessageRepeatableObject obj = (CalendarMessageRepeatableObject)this.removeNextObject(maxDate.getTime() - System.currentTimeMillis());
    return obj == null ? null : obj.getMsg();
  }
}
