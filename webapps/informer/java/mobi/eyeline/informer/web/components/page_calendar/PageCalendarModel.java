package mobi.eyeline.informer.web.components.page_calendar;

import java.util.Date;

/**
 * @author Artem Snopkov
 */
public interface PageCalendarModel {

  void updateVisiblePeriod(Date startDate, Date endDate);

  Object getValue(Date date);
}
