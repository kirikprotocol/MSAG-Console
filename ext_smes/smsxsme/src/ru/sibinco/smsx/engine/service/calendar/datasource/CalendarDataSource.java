package ru.sibinco.smsx.engine.service.calendar.datasource;

import java.util.Collection;
import java.util.Date;

import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: Jun 30, 2007
 */

public interface CalendarDataSource {

  public Collection<CalendarMessage> loadCalendarMessages(Date toDate, int maxSize) throws DataSourceException;
  public CalendarMessage loadCalendarMessageById(int id) throws DataSourceException;
  public void removeCalendarMessage(CalendarMessage calendarMessage) throws DataSourceException;
  public int removeCalendarMessages(Date maxDate, int limit) throws DataSourceException;
  public void saveCalendarMessage(CalendarMessage calendarMessage) throws DataSourceException;
  public void updateMessageStatus(CalendarMessage calendarMessage) throws DataSourceException;
  public int updateMessageStatus(long smppId, int newStatus) throws DataSourceException;
  public void updateMessageSmppId(CalendarMessage calendarMessage) throws DataSourceException;
  public void release();
}
