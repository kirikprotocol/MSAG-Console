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
 * 
 * Changed by miker. This version work without oracle, only on data files.
 */
public class EventsStat
{
  private static final String SMSC_MCISME_STAT_HEADER_TEXT = "SMSC.MCISME.STAT";
  private static final int SMSC_MCISME_STAT_VERSION_INFO = 0x0001;

  private MCISme sme = null;
  private Statistics stat = null;

  public void setMCISme(MCISme sme) {
    this.sme = sme;
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
		  int year	= Integer.parseInt(dirs[i].substring(0,4), 10);
		  int month	= Integer.parseInt(dirs[i].substring(5,7), 10);
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
					  roughFromDate.setHours(fromHour);
					  roughTillDate.setHours(tillHour);
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
						  CountersSet curCounters = new CountersSet();
						  int hour = getNextHourStat(statFile, counters);
						  int curHour = hour;
//						  while( -1 != (hour = getNextHourStat(statFile, counters)))
						  boolean process = true;
						  while(-1 != curHour)
						  {
							  if( hour != curHour )
							  {
                                  curDate.setHours(curHour);
								  if( (curDate.compareTo(roughFromDate) >= 0) && (curDate.compareTo(roughTillDate) <= 0))
								  {
									  HourCountersSet hourStat = new HourCountersSet(curCounters, curHour);
									  dayStat.addHourStat(hourStat);
								  }
								  curHour = hour;
								  curCounters.reset();
							  }
							  else
								  curCounters.increment(counters);
							  if(-1 != hour)
								hour = getNextHourStat(statFile, counters);
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
		  byte[] buff = new byte[17];//[25];
		  if(statFile.read(buff, 0, 17)==-1)  return -1;
		  hour = buff[0];//4];
		  counters.missed = byte4_to_int(buff, 1);//5);
          counters.delivered = byte4_to_int(buff, 5);//);
		  counters.failed = byte4_to_int(buff, 9);//13);
		  counters.notified = byte4_to_int(buff, 13);//17);
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
}
