package ru.sibinco.smppgw.backend.stat.stat;

import javax.sql.DataSource;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 13.08.2004
 * Time: 16:00:56
 * To change this template use File | Settings | File Templates.
 */
public class Stat
{
  private DataSource ds = null;
  private Statistics stat = null;

  public void setDataSource(DataSource ds) {
    this.ds = ds;
  }

  public Statistics getStatistics(StatQuery query) throws Exception
  {
    //TODO: implement query to DB
    //return stat;
    //throw new Exception("Not implemented yet");
    stat = new Statistics();
    for (int i=0; i<5; i++)
    {
      DateCountersSet dcs = new DateCountersSet(new Date());
      SmeIdCountersSet sics = new SmeIdCountersSet(i,i,i,i,i,"SME"+(5-i));
      RouteIdCountersSet rics = new RouteIdCountersSet(i,i,i,i,i,"Route"+(10-i));
      for (int j=0; j<10; j++)
      {
        HourCountersSet hcs = new HourCountersSet(j,j,j,j,j,j);
        ErrorCounterSet ecs = new ErrorCounterSet(j,j);
        dcs.addHourStat(hcs);
        sics.addError(ecs);
        rics.addError(ecs);
      }
      stat.addDateStat(dcs);
      stat.addSmeIdStat(sics);
      stat.addRouteIdStat(rics);
    }
    return stat;
  }

  private static final String PERIOD_DATE_FORMAT = "yyyyMMddHH";

  private boolean needChangeDate(int oldPeriod, int newPeriod)
  {
    String oldStr = Integer.toString(oldPeriod);
    String newStr = Integer.toString(newPeriod);
    oldPeriod = Integer.parseInt(oldStr.substring(0, oldStr.length()-2));
    newPeriod = Integer.parseInt(newStr.substring(0, newStr.length()-2));
    return (oldPeriod != newPeriod);
  }
  private int calculatePeriod(Date date)
  {
    SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
    return Integer.parseInt(formatter.format(date));
  }
  private Date calculateDate(int period) throws Exception
  {
    Date converted = new Date();
    String str = Integer.toString(period);
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
      converted = formatter.parse(str);
    } catch (ParseException e) {
      e.printStackTrace();
      throw new Exception("Failed to convert period to date. "+e.getMessage());
    }
    return converted;
  }
  private int calculateHour(int period)
  {
    String str = Integer.toString(period);
    return Integer.parseInt(str.substring(str.length()-2));
  }

}
