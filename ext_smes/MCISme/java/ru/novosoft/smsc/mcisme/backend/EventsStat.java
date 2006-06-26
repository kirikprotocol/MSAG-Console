package ru.novosoft.smsc.mcisme.backend;

import ru.novosoft.smsc.admin.AdminException;

import javax.sql.DataSource;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Connection;
import java.sql.ResultSet;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.lang.Integer;

import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 18:18:27
 * To change this template use Options | File Templates.
 */
public class EventsStat
{
  private static final String SMSC_MCISME_STAT_HEADER_TEXT = "SMSC.MCISME.STAT";
  private static final int SMSC_MCISME_STAT_VERSION_INFO = 0x0001;

  private static final String PERIOD_DATE_FORMAT = "yyyyMMddHH";
  private static final String STAT_QUERY =
    "SELECT period, sum(missed), sum(delivered), sum(failed), sum(notified) FROM MCISME_STAT ";

  private MCISme sme = null;
  private DataSource ds = null;
  private Statistics stat = null;

  public void setDataSource(DataSource ds) {
    this.ds = ds;
  }
  public void setMCISme(MCISme sme) {
    this.sme = sme;
  }

  private void bindWherePart(PreparedStatement stmt, StatQuery query)
    throws SQLException
  {
    int pos = 1;
    if (query.isFromDateEnabled())
      stmt.setInt(pos++, calculatePeriod(query.getFromDate()));
    if (query.isTillDateEnabled())
      stmt.setInt(pos++, calculatePeriod(query.getTillDate()));
  }

  private String prepareWherePart(StatQuery query)
  {
    String str = (query.isFromDateEnabled() || query.isTillDateEnabled()) ? " WHERE " : "";
    if (query.isFromDateEnabled()) {
      str += " period >= ? ";
      if (query.isTillDateEnabled()) str += " AND ";
    }
    if (query.isTillDateEnabled()) str += " period < ? ";
    return str;
  }

  private String prepareStatQuery(StatQuery query) {
    return STAT_QUERY + prepareWherePart(query) + " GROUP BY period ORDER BY period ASC";
  }

  private void flushStatistics(StatQuery query)
  {
    boolean needFlush = true;
    if (query.isTillDateEnabled()) {
      long till = query.getTillDate().getTime();
      long curr = (new Date()).getTime();
      needFlush = (till >= curr - 3600);
    }
    if (needFlush) {
      try {
        sme.flushStatistics();
      } catch (AdminException e) {
        e.printStackTrace();
      }
    }
  }

  public Statistics getStatistics(String statPath, StatQuery query)
  {
	  Statistics statistics = new Statistics();
	  File	statDir = new File(statPath);

	  try { if(!statDir.exists()) return statistics;}
	  catch(SecurityException e){ return statistics;}

	  String[] dirs = statDir.list();
	  int fromYear = 0, fromMonth = 0, fromDay = 0, fromHour = 0;
	  int tillYear = 3000, tillMonth = 13, tillDay = 32, tillHour = 25;
//	  Date fromDate = new Date(0,0,0,0); 
//	  Date tillDate = new Date(3000,0,0,0); 

	  if(query.isFromDateEnabled())
	  {
		  Date fromDate = query.getFromDate();
		  fromYear = fromDate.getYear() + 1900;
		  fromMonth = fromDate.getMonth()+1;
		  fromDay = fromDate.getDate();
		  fromHour = fromDate.getHours();
	  }
	  if(query.isTillDateEnabled())
	  {
		  Date tillDate = query.getTillDate();
		  tillYear = tillDate.getYear() + 1900;
		  tillMonth = tillDate.getMonth()+1;
		  tillDay = tillDate.getDate();
		  tillHour = tillDate.getHours();
	  }

	  for(int i = 0; i < dirs.length; i++)
	  {
		  int year	= Integer.decode(dirs[i].substring(0,4)).intValue();
		  int month	= Integer.decode(dirs[i].substring(5,7)).intValue();
		  Date curDate = new Date (year-1900, month-1, 1);
		  Date roughFromDate = new Date (fromYear-1900, fromMonth-1, 1);
		  Date roughTillDate = new Date (tillYear-1900, tillMonth-1, 1);

//		  if( (year >= fromYear) && (year <= tillYear) && (month >= fromMonth) && (month <= tillMonth) )
		  if( (curDate.compareTo(roughFromDate) >= 0) && (curDate.compareTo(roughTillDate) <= 0))
		  {
			  File dirDays = new File(statDir, dirs[i]);
			  String[] days = dirDays.list();
			  for(int j = 0; j < days.length; j++)
			  {
                  int day = Integer.parseInt(days[j].substring(0,2), 10);
				  curDate.setDate(day);
				  roughFromDate.setDate(fromDay);
				  roughTillDate.setDate(tillDay);

//				  if( (day >= fromDay) && (day <= tillDay))
				  if( (curDate.compareTo(roughFromDate) >= 0) && (curDate.compareTo(roughTillDate) <= 0))
				  {
					  Date dt = new Date(year-1900, month-1, day);
					  DateCountersSet dayStat = new DateCountersSet(dt);
					  try
					  {
						  File dayFile = new File(dirDays, days[j]);
						  FileInputStream statFile = new FileInputStream(dayFile);
						  byte[] buff = new byte[SMSC_MCISME_STAT_HEADER_TEXT.length()];
						  statFile.read(buff, 0, SMSC_MCISME_STAT_HEADER_TEXT.length());
						  String header = new String(buff);
						  if(SMSC_MCISME_STAT_HEADER_TEXT.compareTo(header) != 0)  continue;
						  statFile.read(buff, 0, 2);
						  int version = buff[0]; version = (version << 8) | buff[1];
						  if(version != SMSC_MCISME_STAT_VERSION_INFO)  continue;
						  CountersSet counters = new CountersSet();
						  int hour=0;
						  while( -1 != (hour = getNextHourStat(statFile, counters)))
						  {
							  curDate.setHours(hour);
							  roughFromDate.setHours(fromHour);
							  roughTillDate.setHours(tillHour);
							  
//							  if( (hour >= fromHour) && (hour <= tillHour) )
							  if( (curDate.compareTo(roughFromDate) >= 0) && (curDate.compareTo(roughTillDate) <= 0))
							  {
								  HourCountersSet hourStat = new HourCountersSet(counters, hour);
								  dayStat.addHourStat(hourStat);
							  }
						  }
						  statistics.addDateStat(dayStat);
						  statFile.close();
					  }
					  catch(Exception e)
					  {

					  }
				  }
			  }
		  }
	  }
	  return statistics;
  }
  
  private int getNextHourStat(FileInputStream statFile, CountersSet counters)
  {
	  int hour;
	  try
	  {
		  byte[] buff = new byte[25];
		  if(statFile.read(buff, 0, 25)==-1)  return -1;
		  hour = buff[4];
		  counters.missed = byte4_to_int(buff, 5);
          counters.delivered = byte4_to_int(buff, 9);
		  counters.failed = byte4_to_int(buff, 13);
		  counters.notified = byte4_to_int(buff, 17);
	  }
	  catch(Exception e)
	  {
		  return -1;
	  }
	  return hour;
  }
  private long byte4_to_int(byte[] arr, int from)
  {
	  long res = 0;
	  for(int i = 0; i < 4; i++)
		res = add_byte(res, arr[from+i]);
	  return res;
  }
  private long add_byte(long value, byte num)
  {
	  long res = 0;
	  if(num < 0)
	  {
		  res = (value << 8) | (num & 0x7F);
		  res = res | 0x80;
	  }
	  else
		  res = res = (value << 8) | num;
	  
	  return res;	
  }
  private PreparedStatement getQuery(Connection connection, StatQuery query, String sql)
    throws SQLException
  {
    PreparedStatement stmt = connection.prepareStatement(sql);
    bindWherePart(stmt, query);
    return stmt;
  }

  private void processStatQuery(Connection connection, StatQuery query)
    throws SQLException
  {
    int oldPeriod = 0;
    DateCountersSet dateCounters = null;
    PreparedStatement stmt = getQuery(connection, query, prepareStatQuery(query));
    ResultSet rs = stmt.executeQuery();

    try {
      while (rs.next()) {
        int newPeriod = rs.getInt(1);
        int hour = calculateHour(newPeriod);
        HourCountersSet hourCounters = new HourCountersSet(
                rs.getInt(2), rs.getInt(3), rs.getInt(4), rs.getInt(5), hour);
        if (dateCounters == null) { // on first iteration
          Date date = calculateDate(newPeriod);
          dateCounters = new DateCountersSet(date);
        } else if (needChangeDate(oldPeriod, newPeriod)) { // on date changed
          stat.addDateStat(dateCounters);
          Date date = calculateDate(newPeriod);
          dateCounters = new DateCountersSet(date);
        }
        dateCounters.addHourStat(hourCounters);
        oldPeriod = newPeriod;
      }

      if (dateCounters != null) stat.addDateStat(dateCounters);

    } catch (SQLException ex) {
      throw ex;
    } finally {
      if (rs != null) rs.close();
      if (stmt != null) stmt.close();
    }
  }

  private boolean needChangeDate(int oldPeriod, int newPeriod)
  {
    String oldStr = Integer.toString(oldPeriod);
    String newStr = Integer.toString(newPeriod);
    oldPeriod = Integer.parseInt(oldStr.substring(0, oldStr.length() - 2));
    newPeriod = Integer.parseInt(newStr.substring(0, newStr.length() - 2));
    return (oldPeriod != newPeriod);
  }

  private int calculatePeriod(Date date)
  {
    SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
    return Integer.parseInt(formatter.format(date));
  }

  private Date calculateDate(int period)
  {
    Date converted = new Date();
    String str = Integer.toString(period);
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
      converted = formatter.parse(str);
    } catch (ParseException e) {
      e.printStackTrace();
    }
    return converted;
  }

  private int calculateHour(int period)
  {
    String str = Integer.toString(period);
    return Integer.parseInt(str.substring(str.length() - 2));
  }

}
