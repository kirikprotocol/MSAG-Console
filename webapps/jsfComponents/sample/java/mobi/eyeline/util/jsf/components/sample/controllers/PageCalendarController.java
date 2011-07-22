package mobi.eyeline.util.jsf.components.sample.controllers;

import mobi.eyeline.util.jsf.components.page_calendar.PageCalendarModel;

import java.util.Date;

/**
 * author: Aleksandr Khalitov
 */
public class PageCalendarController {


  public PageCalendarModel getCalendarModel() {

    return new PageCalendarModel() {

      public void updateVisiblePeriod(Date startDate, Date endDate) {
      }

      public Object getValue(Date date) {
        return date.toString();
      }
    };
  }
}
