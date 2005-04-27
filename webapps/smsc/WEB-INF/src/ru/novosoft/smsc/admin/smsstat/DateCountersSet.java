package ru.novosoft.smsc.admin.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:37:26 PM
 * To change this template use Options | File Templates.
 */

import java.util.Collection;
import java.util.Date;
import java.util.Vector;

public class DateCountersSet extends CountersSet
{
  private Date date;
  private int dateFile;
  private Vector byHours = new Vector(); // contains HourCountersSet

  DateCountersSet(Date date)
  {
    this.date = date;
  }
     DateCountersSet(Date date,int dateFile)
  {
    this.date = date;
    this.dateFile = dateFile;
  }
  public void addHourStat(HourCountersSet set)
  {
    byHours.addElement(set);
    super.increment(set);
  }

  public Collection getHourStat()
  {
    return byHours;
  }

  public Date getDate()
  {
    return date;
  }

  public int getDateFile()
  {
    return dateFile;
  }
}
