package ru.novosoft.smsc.admin.smsstat;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsview.DateConvertor;

import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.EOFException;
import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.text.NumberFormat;


/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 24.04.2005
 * Time: 21:14:19
 * To change this template use File | Settings | File Templates.
 */
public class StatFileReader
{
  org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(StatFileReader.class);
  private boolean allReaded=false;
  private Statistics statTemp  = new Statistics();
  private Statistics stat = new Statistics();
  private static final String PERIOD_DATE_HOUR_FORMAT = "yyyyMMddHH";
  private static final String PERIOD_DATE_FORMAT = "yyyyMMdd";
  private static final String PERIOD_HOUR_FORMAT = "HH";
  private DateCountersSet dateCounters = null;
  private HourCountersSet hourCounters=null;
  private HashMap countersForSme = new HashMap();
  private HashMap countersForRoute = new HashMap();
  private int hourLocal=0;
  private HashSet warnRoutId=new HashSet();
  private HashMap warnProvId=new HashMap();
  private HashMap warnCatId=new HashMap();
  protected RouteSubjectManager routeSubjectManager = null;
  protected ProviderManager providerManager = null;
  protected CategoryManager categoryManager = null;
  public StatFileReader(RouteSubjectManager routeSubjectManager,ProviderManager providerManager,
                         CategoryManager categoryManager,HashSet warnRoutId,HashMap warnProvId,
                         HashMap warnCatId,Statistics stat,HashMap countersForSme,HashMap countersForRoute)
  {
    this.routeSubjectManager=routeSubjectManager;
    this.providerManager=providerManager;
    this.categoryManager=categoryManager;
    this.warnRoutId=warnRoutId;
    this.warnProvId=warnProvId;
    this.warnCatId=warnCatId;
    this.stat=stat;
    this.countersForSme=countersForSme;
    this.countersForRoute=countersForRoute;
    this.dateCounters=null;
  }

  public StatFileReader(RouteSubjectManager routeSubjectManager,ProviderManager providerManager,
                         CategoryManager categoryManager,HashSet warnRoutId,HashMap warnProvId,
                         HashMap warnCatId,Statistics stat,HashMap countersForSme,
                         HashMap countersForRoute,DateCountersSet dateCounters)
  {
    this.routeSubjectManager=routeSubjectManager;
    this.providerManager=providerManager;
    this.categoryManager=categoryManager;
    this.warnRoutId=warnRoutId;
    this.warnProvId=warnProvId;
    this.warnCatId=warnCatId;
    this.stat=stat;
    this.countersForSme=countersForSme;
    this.countersForRoute=countersForRoute;
    this.dateCounters=dateCounters;
  }
  public static int readUInt8(InputStream is) throws IOException
  {
    int b = is.read();
    if (b == -1) throw new EOFException();
    return b;
  }

  public static int readUInt16(InputStream is) throws IOException
  {
    return (readUInt8(is) << 8 | readUInt8(is));
  }

  public static long readUInt32(InputStream is) throws IOException
  {
    return ((long) readUInt8(is) << 24) | ((long) readUInt8(is) << 16) |
            ((long) readUInt8(is) << 8) | ((long) readUInt8(is));
  }

  public static long readInt64(InputStream is) throws IOException
  {
    return (readUInt32(is) << 32) | readUInt32(is);
  }

  public static String readString(InputStream is, int size) throws IOException
  {
    if (size <= 0) return "";
    byte buff[] = new byte[size];
    int pos = 0;
    int cnt = 0;
    while (pos < size) {
      cnt = is.read(buff, pos, size - pos);
      if (cnt == -1) throw new EOFException();
      pos += cnt;
    }
    return new String(buff);
  }

/*  public static String readString8(InputStream is) throws IOException
  {
    return readString(is, readUInt8(is));
  }

  public static String readString16(BufferedInputStream is) throws IOException
  {
    return readString(is, readUInt16(is));
  }

  public static String readString32(InputStream is) throws IOException
  {
    return readString(is, (int) readUInt32(is));
  }
 */
  public  Statistics receiveStat(InputStream bis , Date dateFile) throws AdminException ,IOException
  {

    int msgSize2=-1;
    while (!allReaded) {
      msgSize2=receiveRecord(bis , dateFile);
      if (!allReaded){
//              stat.addRecord(statTemp);// скипание последней оборванной записи
      }
      else {
        if (msgSize2==-1) logger.warn("last record was not finished");// пишем ворнинг
      }
    }
    return stat;
  }

  public  int receiveRecord(InputStream bis , Date dateFile) throws AdminException ,IOException
  {

    int msgSize1=0;int msgSize2=0;
    try {
      msgSize1 = (int)readUInt32(bis);
      processSmsQuery(bis , dateFile);
      processStateQuery(bis);
      processSmeQuery(bis);
      processRouteQuery(bis);

      msgSize2 = (int) StatFileReader.readUInt32(bis);
      if (msgSize1 != msgSize2) {  msgSize2=-1;
        System.out.println("error sz1 sz2");
        throw new IOException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);
      }
    } catch (EOFException e) {
      this.setAllReaded(true);//break;
      if (msgSize1 != msgSize2) msgSize2=-1;
      dateCounters.addHourStat(hourCounters);
      //  if (dateCounters != null  ) stat.addDateStat(dateCounters);

    }
    return msgSize2;
  }
  private void processSmsQuery(InputStream bis, Date dateFile)
          throws IOException, AdminException
  {
    int hour = (int)readUInt8(bis);
    int min = (int)readUInt8(bis);

    long timeMillisGMT = dateFile.getTime()+hour*3600000+min*60000;

    SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_HOUR_FORMAT);
    formatter.setTimeZone(TimeZone.getDefault());
    Date timeLocal=new Date(timeMillisGMT);
    String timeFileLOCAL=formatter.format(timeLocal);
    String timeLOCAL=timeFileLOCAL.substring(8,10);
    String dateLOCAL=timeFileLOCAL.substring(0,8);
    formatter.applyPattern(PERIOD_DATE_FORMAT);

    int hourLocal=Integer.parseInt(timeLOCAL);
    //int dateGmt=Integer.parseInt(dateFileGmt);
    int dateLocal=Integer.parseInt(dateLOCAL);

    // if (dateGmt!=dateLocal) needChangeDate=true;

    int accepted = (int)readUInt32(bis);
    int rejected = (int)readUInt32(bis);
    int delivered = (int)readUInt32(bis);
    int failed  = (int)readUInt32(bis);
    int rescheduled = (int)readUInt32(bis);
    int temporal = (int)readUInt32(bis);
    int peak_i = (int)readUInt32(bis);
    int peak_o = (int)readUInt32(bis);
    if (dateCounters == null) { // on first iteration
      dateCounters = new DateCountersSet(dateFile,dateLocal);
    }
    if (hourLocal!=this.hourLocal  && hourLocal==0  && hourCounters!=null) { // on date changed  (needChangeDate(oldPeriod, timeMillisGMT))
      dateCounters.addHourStat(hourCounters);
      stat.addDateStat(dateCounters);
      //Date date = calculateDate(timeLocal.getTime());
      dateCounters = new DateCountersSet(timeLocal,dateLocal);
    }

    if (hourLocal!=this.hourLocal ||hourCounters==null) {
      //if (filledHour) // filledHour=false;
      if (hourCounters!=null && hourLocal!=0 ) dateCounters.addHourStat(hourCounters);
      hourCounters = new HourCountersSet(accepted , rejected ,delivered ,failed
              ,rescheduled ,temporal ,peak_i,peak_o, hourLocal);
    }
    else {
      CountersSet counters=new CountersSet(accepted , rejected ,delivered ,failed
              ,rescheduled ,temporal ,peak_i,peak_o);
      hourCounters.increment(counters);
    }

    this.hourLocal=hourLocal;
  }

  private void processStateQuery(InputStream bis )
          throws IOException
  {
    int counter = (int)readUInt32(bis);
    while (counter>0) {
      int errcode = (int)readUInt32(bis);
      int count = (int)readUInt32(bis);
      counter--;
      ErrorCounterSet errorCounterSet=stat.getErrStat(errcode);
      if(errorCounterSet!=null) {
        errorCounterSet.increment(count);
        stat.putErrorStat(errorCounterSet);
      }
      else stat.putErrorStat(new ErrorCounterSet(errcode, count));

    }
  }
  private void processSmeQuery(InputStream bis )
          throws IOException
  {


    int counter = (int)readUInt32(bis);
    while (counter>0)
    {
      int sme_id_len = (int)readUInt8(bis);
      String smeId = readString(bis,sme_id_len);
      int accepted = (int)readUInt32(bis);
      int rejected = (int)readUInt32(bis);
      int delivered = (int)readUInt32(bis);
      int failed  = (int)readUInt32(bis);
      int rescheduled = (int)readUInt32(bis);
      int temporal = (int)readUInt32(bis);
      int peak_i = (int)readUInt32(bis);
      int peak_o = (int)readUInt32(bis);

      if ( smeId == null) continue;
      SmeIdCountersSet smeIdCountersSet= (SmeIdCountersSet)countersForSme.get(smeId);
      if (smeIdCountersSet!=null) {
        CountersSet countSme=new CountersSet(accepted , rejected ,delivered ,failed
                ,rescheduled ,temporal ,peak_i,peak_o);
        smeIdCountersSet.increment(countSme);
        countersForSme.put(smeId,smeIdCountersSet);
      }
      else countersForSme.put(smeId, new SmeIdCountersSet( accepted , rejected ,delivered ,failed ,rescheduled
              ,temporal ,peak_i,peak_o, smeId));

      int err_counter = (int)readUInt32(bis);
      while (err_counter>0) {
        int errcode = (int)readUInt32(bis);
        int count = (int)readUInt32(bis);
        err_counter--;

        Object obj = countersForSme.get(smeId);
        if (obj != null && obj instanceof SmeIdCountersSet) {
          SmeIdCountersSet smeIdCounterSet=(SmeIdCountersSet)obj;
          ErrorCounterSet errorCounterSet=smeIdCounterSet.getErr(errcode);
          if(errorCounterSet!=null) {
            errorCounterSet.increment(count);
            smeIdCounterSet.putErr(errcode,errorCounterSet);
          }
          else  smeIdCounterSet.putErr(errcode,new ErrorCounterSet(errcode, count));

        }
      }
      counter--;
    }

  }

  private void processRouteQuery(InputStream bis)
          throws IOException
  {


    int counter = (int)readUInt32(bis);
    while (counter>0)
    {
      int route_id_len = (int)readUInt8(bis);
      String routeId = readString(bis,route_id_len);
      long providerId = readInt64(bis);
      long categoryId = readInt64(bis);
      int accepted = (int)readUInt32(bis);
      int rejected = (int)readUInt32(bis);
      int delivered = (int)readUInt32(bis);
      int failed  = (int)readUInt32(bis);
      int rescheduled = (int)readUInt32(bis);
      int temporal = (int)readUInt32(bis);
      int peak_i = (int)readUInt32(bis);
      int peak_o = (int)readUInt32(bis);

      Provider provider = null; Category category = null;
      final Route r =  routeSubjectManager.getRoutes().get(routeId);
      if (r == null) {
        warnRoutId.add(routeId);// logger.warn("Route '"+routeId+"' is undefined");
        if (categoryId != -1) category = categoryManager.getCategory(new Long(categoryId));
        if (providerId != -1) provider = providerManager.getProvider(new Long(providerId));
      }
      else {
        if (providerId == -1) provider = null;
        else if (r.getProviderId() != providerId) {
          warnProvId.put(routeId,new Long(providerId));//logger.warn("ProviderId='"+providerId+"' doesn't match for route '"+routeId+"'  ");
          provider = new Provider(providerId, "undefined ("+providerId+")");
        }
        else provider = providerManager.getProvider(new Long(providerId));
        if (categoryId == -1) category = null;
        else if (r.getCategoryId() != categoryId) {
          warnCatId.put(routeId,new Long(categoryId));//logger.warn("CategoryId='"+providerId+"' doesn't match for route '"+routeId+"'  ");
          category = new Category(categoryId, "undefined ("+categoryId+")");
        }
        else category = categoryManager.getCategory(new Long(categoryId));
      }
      if (provider == null) provider = new Provider(-1, "");
      if (category == null) category = new Category(-1, "");
      
      RouteIdCountersSet routeIdCountersSet=(RouteIdCountersSet)countersForRoute.get(routeId);
      if (routeIdCountersSet!=null) {
        CountersSet countRoute=new CountersSet(accepted , rejected ,delivered ,failed
                ,rescheduled ,temporal ,peak_i,peak_o);
        routeIdCountersSet.increment(countRoute);
        countersForRoute.put(routeId,routeIdCountersSet);
      }
      else   countersForRoute.put(routeId,
              new RouteIdCountersSet(accepted , rejected ,delivered ,failed ,rescheduled
                      ,temporal ,peak_i,peak_o, routeId, provider, category));

      int err_counter = (int)readUInt32(bis);
      while (err_counter>0) {
        int errcode = (int)readUInt32(bis);
        int count = (int)readUInt32(bis);
        err_counter--;

        Object obj = countersForRoute.get(routeId);
        if (obj != null && obj instanceof RouteIdCountersSet) {
          RouteIdCountersSet routeIdCounterSet=(RouteIdCountersSet)obj;
          ErrorCounterSet errorCounterSet=routeIdCounterSet.getErr(errcode);
          if(errorCounterSet!=null) {
            errorCounterSet.increment(count);
            routeIdCounterSet.putErr(errcode,errorCounterSet);
          }
          else routeIdCounterSet.putErr(errcode,new ErrorCounterSet(errcode, count));

        }
      }
      counter--;
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
  private boolean needChangeHour( int min)
  {
    return (min == 0);
  }


  private Date calculateDate(long period) throws AdminException
  {
    SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_DATE_FORMAT);
    formatter.setTimeZone(TimeZone.getDefault());
    Date timeLocal=new Date(period);
    return timeLocal;
  }
  private int reciveGMTHour(int period)
  {
    String str = Integer.toString(period);
    SimpleDateFormat formatter = new SimpleDateFormat(PERIOD_HOUR_FORMAT);
    formatter.setTimeZone(TimeZone.getDefault());
    String hours=formatter.format( new Date(period));
    int hour=Integer.parseInt(hours);

    return hour;
  }

  private int calculateHour(int period)
  {
    String str = Integer.toString(period);
    return Integer.parseInt(str.substring(str.length() - 2));
  }
  public boolean isAllReaded() {
    return allReaded;
  }

  public void setAllReaded(boolean allReaded) {
    this.allReaded = allReaded;
  }

  public DateCountersSet getDateCounters()
  {
    return dateCounters;
  }

  public HashMap getCountersForSme()
  {
    return countersForSme;
  }

  public HashMap getCountersForRoute()
  {
    return countersForRoute;
  }

  public HashMap getWarnCatId()
  {
    return warnCatId;
  }

  public HashMap getWarnProvId()
  {
    return warnProvId;
  }

  public HashSet getWarnRoutId()
  {
    return warnRoutId;
  }
}
