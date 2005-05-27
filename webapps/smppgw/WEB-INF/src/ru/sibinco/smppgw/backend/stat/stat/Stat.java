package ru.sibinco.smppgw.backend.stat.stat;

import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.backend.util.WebAppFolders;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.smppgw.beans.MessageException;
import ru.sibinco.smppgw.backend.SmppGWAppContext;
import ru.sibinco.smppgw.backend.Constants;

import javax.sql.DataSource;
import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.io.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 13.08.2004
 * Time: 16:00:56
 * To change this template use File | Settings | File Templates.
 */
public class Stat
{
  /*
  private final static String ERRORS_SET =
  " errcode, sum(counter) ";
  private final static String VALUES_SET =
  " sum(accepted), sum(rejected), sum(delivered), sum(temperror), sum(permerror) ";
  private final static String TRANS_VALUES_SET =
  " sum(SmsTrOk), sum(SmsTrFailed), sum(SmsTrBilled), sum(UssdTrFromScOk), sum(UssdTrFromScFailed),"+
  " sum(UssdTrFromScBilled), sum(UssdTrFromSmeOk), sum(UssdTrFromSmeFailed), sum(UssdTrFromSmeBilled) ";

  private final static String TOTAL_QUERY =   // group by period
  "SELECT period,"+VALUES_SET+","+TRANS_VALUES_SET+"FROM smppgw_stat_sme ";
  private final static String SME_QUERY =     // group by systemid
  "SELECT systemid,"+VALUES_SET+","+TRANS_VALUES_SET+"FROM smppgw_stat_sme ";
  private static final String ROUTE_QUERY =   // group by routeid
  "SELECT routeid,"+VALUES_SET+"FROM smppgw_stat_route ";

  private static final String SME_STATE_QUERY =   // group by systemid, errcode
  "SELECT systemid,"+ERRORS_SET+"FROM smppgw_stat_sme_errors ";
  private static final String ROUTE_STATE_QUERY = // group by routeid, errcode
  "SELECT routeid,"+ERRORS_SET+"FROM smppgw_stat_route_errors ";
  */
  org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(Stat.class);

  private final static String DATE_DIR_FORMAT = "yyyy-MM";
  private final static String DATE_DAY_FORMAT = "yyyy-MM-dd hh:mm";
  private final static String DATE_DIR_FILE_FORMAT = DATE_DIR_FORMAT + File.separatorChar + "dd";
  private final static String DATE_FILE_EXTENSION  = ".rts";
 // private final static String DATE_PERIOD_FORMAT = "yyyyMMddhhmm";

  private Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
 // private Calendar localCaledar = Calendar.getInstance(TimeZone.getDefault());
  private SimpleDateFormat dateDirFormat = new SimpleDateFormat(DATE_DIR_FORMAT);
  private SimpleDateFormat dateDirFileFormat = new SimpleDateFormat(DATE_DIR_FILE_FORMAT);
  private SimpleDateFormat dateDayFormat = new SimpleDateFormat(DATE_DAY_FORMAT);
  private SimpleDateFormat dateDayLocalFormat = new SimpleDateFormat(DATE_DAY_FORMAT);
//  private SimpleDateFormat datePeriodLocalFormat = new SimpleDateFormat(DATE_PERIOD_FORMAT);

  private String statstorePath;
  private final static String PARAM_NAME_STAT_DIR = "MessageStorage.statisticsDir";

  private Date fromQueryDate = null;
  private Date tillQueryDate = null;

//  private DataSource ds = null;
  private Statistics stat = null;

/*  public void setDataSource(DataSource ds) {
this.ds = ds;
}
*/
  private static Object instanceLock = new Object();
  private static Stat instance = null;

  public static Stat getInstance(Config gwConfig ) throws Exception
  {
    synchronized(instanceLock) {
      if (instance == null) instance = new Stat( gwConfig );
      return instance;
    }
  }

  protected Stat(Config gwConfig ) throws MessageException
  {
    try {
      statstorePath = gwConfig.getString(PARAM_NAME_STAT_DIR);
      if (statstorePath == null || statstorePath.length() <= 0)
        throw new MessageException("store path is empty");
    } catch (Exception e) {
      throw new MessageException("Failed to obtain statistics dir. Details: " + e.getMessage());
    }
    dateDirFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
    dateDirFileFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
    dateDayFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
    dateDayLocalFormat.setTimeZone(TimeZone.getDefault());
   // datePeriodLocalFormat.setTimeZone(TimeZone.getDefault());
  }

  private TreeMap getStatQueryDirs() throws MessageException
  {
    File statPath = new File(statstorePath);
    if (statPath == null || !statPath.isAbsolute()) {
      File gwConfFile   = SmppGWAppContext.getGwConfFolder();
      // File smscConfFile = WebAppFolders.getSmscConfFolder();
      String gwDirFile = gwConfFile.getParent();
      statPath = new File(gwDirFile, statstorePath);
      logger.debug("Stat path: by gateway conf '"+statPath.getAbsolutePath()+"'");
    } else {
      logger.debug("Stat path: is absolute '"+statPath.getAbsolutePath()+"'");
    }

    String[] dirNames = statPath.list();
    if (dirNames == null || dirNames.length == 0)
      throw new MessageException("No stat directories at path '"+statPath.getAbsolutePath()+"'");

    Date tillQueryDirTime = tillQueryDate;
    Date tillQueryFileTime = tillQueryDate;
    Date fromQueryDirTime = null;
    Date fromQueryFileTime = null;
    if (fromQueryDate != null) {
      calendar.setTime(fromQueryDate);
      calendar.set(Calendar.HOUR_OF_DAY, 0);
      calendar.set(Calendar.MINUTE, 0);
      calendar.set(Calendar.MILLISECOND, 0);
      fromQueryFileTime = calendar.getTime();
      calendar.set(Calendar.DAY_OF_MONTH, 0);
      fromQueryDirTime = calendar.getTime();
    }

    TreeMap selected = new TreeMap();
    for (int i = 0; i < dirNames.length; i++) {
      String dirName = dirNames[i];
      if (dirName == null || dirName.length() <= 0) continue;

      Date dirDate;
      try { dirDate = dateDirFormat.parse(dirName); }
      catch (ParseException exc) { continue; }

      if (fromQueryDirTime != null && dirDate.getTime() < fromQueryDirTime.getTime()) continue;
      if (tillQueryDirTime != null && dirDate.getTime() > tillQueryDirTime.getTime()) continue;

      File dirNameFile = new File(statPath, dirName);
      File[] dirFiles = dirNameFile.listFiles();
      //logger.debug("Stat path: dir '" + dirNameFile.getAbsolutePath() + "'");
      if (dirFiles == null || dirFiles.length == 0) continue;

      for (int j = 0; j < dirFiles.length; j++) {
        String fileName = dirFiles[j].getName();
        if (fileName == null || fileName.length() <= 0 ||
                !fileName.toLowerCase().endsWith(DATE_FILE_EXTENSION)) continue;

        Date fileDate;
        try { fileDate = dateDirFileFormat.parse(dirName + File.separatorChar + fileName); }
        catch (ParseException exc) { continue; }

        if (fromQueryFileTime != null && fileDate.getTime() < fromQueryFileTime.getTime()) continue;
        if (tillQueryFileTime != null && fileDate.getTime() > tillQueryFileTime.getTime()) continue;

        final String fullFilePath = (new File(dirNameFile, fileName)).getAbsolutePath();
        selected.put(fileDate, fullFilePath);
      }
    }
    return selected;
  }

  /*
  private Statistics getFakeStatistics(StatQuery query) throws Exception
  {
  stat = new Statistics();
  for (int i=0; i<5; i++)
  {
  DateCountersSet dcs = new DateCountersSet(new Date());
  SmeIdCountersSet sics = new SmeIdCountersSet(i,i,i,i,i,i,i,i,i,i,i,i,i,i,"SME"+(5-i));
  RouteIdCountersSet rics = new RouteIdCountersSet(i,i,i,i,i,"Route"+(10-i));
  for (int j=0; j<3; j++)
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
  }*/

  private Object generalLock = new Object();

  public Statistics getStatistics(StatQuery query) throws Exception
  {
    synchronized(generalLock) {
      return _getStatistics(query);
    }
  }
  private void initQueryPeriod(StatQuery query) {
    fromQueryDate = query.isFromDateEnabled() ? query.getFromDate() : null;
    tillQueryDate = query.isTillDateEnabled() ? query.getTillDate() : null;
  }

  private Statistics _getStatistics(StatQuery query) throws Exception
  {

    initQueryPeriod(query);
    if (logger.isInfoEnabled())
    {
      String fromDate = " -";
      if (fromQueryDate != null) {
        fromDate = " from " + dateDayLocalFormat.format(fromQueryDate);
        fromDate += " (" + dateDayFormat.format(fromQueryDate) + " GMT)";
      }
      String tillDate = " -";
      if (tillQueryDate != null) {
        tillDate = " till " + dateDayLocalFormat.format(tillQueryDate);
        tillDate += " (" + dateDayFormat.format(tillQueryDate) + " GMT)";
      }
      logger.info("Query stat" + fromDate + tillDate);
    }

    Statistics stat = new Statistics();
    TreeMap selectedFiles = getStatQueryDirs();
    if (selectedFiles == null || selectedFiles.size() <= 0) return stat;

    //TreeMap statByHours = new TreeMap(); // to add lastHourCounter to it
    HashMap countersForSme = new HashMap(); // contains SmeIdCountersSet
    HashMap countersForRoute = new HashMap(); // contains RouteIdCountersSet


    long tm = System.currentTimeMillis();  boolean finished = false;
        for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext() && !finished;) {
            Date fileDate = (Date) iterator.next(); // GMT
            String path = (String) selectedFiles.get(fileDate);
            {
                logger.debug("Parsing file: "+dateDayFormat.format(fileDate)+" GMT ("+dateDayLocalFormat.format(fileDate)+" local)");
            }
            InputStream input = null;
            try {
                input = new BufferedInputStream(new FileInputStream(path));
                String fileStamp = readString(input, 10);
                if (fileStamp == null || !fileStamp.equals("SMPPGW.STAT"))
                    throw new MessageException("unsupported header of file (support only SMPPGW.STAT file )");
                readUInt16(input); // read version for support reasons
                //CountersSet lastHourCounter = new CountersSet();
             //   Date lastDate = null;
                Date curDate = null;
             //   int prevHour = -1;
                byte buffer[] = new byte[512 * 1024];
              //  boolean haveValues = false;
                int recordNum = 0;
                while (true) // iterate file records (by minutes)
                {
                    try {
                        recordNum++;
                        int rs1 = (int) readUInt32(input);
                        if (buffer.length < rs1) buffer = new byte[rs1];
                        Functions.readBuffer(input,buffer,rs1);
                        int rs2 = (int) readUInt32(input);
                        if (rs1 != rs2)
                            throw new IOException("Invalid file format "+path+" rs1=" + rs1 + ", rs2=" + rs2+" at record="+recordNum);

                        ByteArrayInputStream is = new ByteArrayInputStream(buffer, 0, rs1);
                        try {
                            int hour = readUInt8(is);
                            int min = readUInt8(is);
                            calendar.setTime(fileDate);
                            calendar.set(Calendar.HOUR, hour);
                            calendar.set(Calendar.MINUTE, min);
                            curDate = calendar.getTime();

                            if (fromQueryDate != null && curDate.getTime() < fromQueryDate.getTime()) {
                                logger.debug("Hour: "+hour+" skipped");
                                continue;
                            }

                       //     if (prevHour == -1) prevHour = hour;
                       //     if (lastDate == null) lastDate = curDate;

                      /*      if (hour != prevHour && haveValues) { // switch to new hour
                                logger.debug("New hour: " + hour + ", dump stat for: " + dateDayFormat.format(lastDate) + " GMT");
                               // statByHours.put(lastDate, lastHourCounter);
                                haveValues = false;
                                lastDate = curDate;
                                prevHour = hour;
                              //  lastHourCounter = new CountersSet();
                            }  */

                            if (tillQueryDate != null && curDate.getTime() >= tillQueryDate.getTime()) {
                                finished = true;
                                break; // finish work
                            }

                     //       haveValues = true; // read and increase counters
                          //  scanCounters(lastHourCounter, is);
                          //  scanErrors(stat, is);
                            scanRoutes(countersForRoute, is);
                            scanSmes(countersForSme, is);
                        } catch (EOFException exc) {
                            logger.warn("Incomplete record #"+recordNum+" in "+path+"");
                        }
                    } catch (EOFException exc) {
                        break;
                    }
                }
               /* if (haveValues) {
                    logger.debug("Last dump stat for: " + dateDayFormat.format(lastDate) + " GMT");
                    statByHours.put(lastDate, lastHourCounter);
                }  */

            } catch (IOException e) {
                throw new MessageException(e.getMessage());
            } finally {
                try {
                    if (input != null) input.close();
                } catch (Throwable th) {
                    th.printStackTrace();
                }
            }
        } // for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext();)
        logger.debug("End scanning statistics at: " + new Date() + " time spent: " +
                     (System.currentTimeMillis() - tm) / 1000);
  /*
        DateCountersSet dateCounters = null;
        Date lastDate = null;
        for (Iterator it = statByHours.keySet().iterator(); it.hasNext();)
        {
            Date hourDate = (Date) it.next();
            CountersSet hourCounter = (CountersSet) statByHours.get(hourDate);
            localCaledar.setTime(hourDate);
            int hour = localCaledar.get(Calendar.HOUR_OF_DAY);

            if (lastDate == null || (hourDate.getTime() - lastDate.getTime()) >= Constants.Day) {
                localCaledar.set(Calendar.HOUR_OF_DAY, 0);
                localCaledar.set(Calendar.MINUTE, 0);
                localCaledar.set(Calendar.SECOND, 0);
                localCaledar.set(Calendar.MILLISECOND, 0);
                lastDate = localCaledar.getTime();
                if (dateCounters != null) stat.addDateStat(dateCounters);
                dateCounters = new DateCountersSet(lastDate);
            }

            HourCountersSet set = new HourCountersSet(hour);
            set.increment(hourCounter);
            dateCounters.addHourStat(set);
        }
        if (dateCounters != null) stat.addDateStat(dateCounters);
    */
        Collection countersSme = countersForSme.values();
        if (countersSme != null) stat.addSmeIdCollection(countersSme);
        Collection countersRoute = countersForRoute.values();
        if (countersRoute != null) stat.addRouteIdCollection(countersRoute);

        return stat;


 /*
//    if (ds == null) throw new Exception("DataSource is not initialized");
    //   Connection connection = null;
    stat = new Statistics();
    try
    {
      //    connection = ds.getConnection();
      processSmeQuery(connection, query);
      processRouteQuery(connection, query);
    }
    catch (SQLException exc) {
      final String errorMessage = "Operation with DB failed !";
      System.out.println(errorMessage); exc.printStackTrace();
      throw new Exception(errorMessage+" Details: "+exc.getMessage());
    } finally {
      try { if (connection != null) connection.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
    }
    return stat;
    //return getFakeStatistics(query);
  */
  }

  private void scanCounters(CountersSet set, InputStream is) throws IOException {
         int accepted = (int) readUInt32(is);
         int rejected = (int) readUInt32(is);
         int delivered = (int) readUInt32(is);
         int temperror = (int) readUInt32(is);
         int permerror = (int) readUInt32(is);
         set.increment(accepted, rejected, delivered, temperror,  permerror);
     }

   private void scanSmeCounters(CountersSet set, InputStream is) throws IOException {
     int accepted = (int) readUInt32(is);
     int rejected = (int) readUInt32(is);
     int delivered = (int) readUInt32(is);
     int temperror = (int) readUInt32(is);
     int permerror = (int) readUInt32(is);

     int SmsTrOk             = (int) readUInt32(is);
     int SmsTrFailed         = (int) readUInt32(is);
     int SmsTrBilled         = (int) readUInt32(is);
     int UssdTrFromScOk      = (int) readUInt32(is);
     int UssdTrFromScFailed  = (int) readUInt32(is);
     int UssdTrFromSmeFailed = (int) readUInt32(is);
     int UssdTrFromSmeBilled = (int) readUInt32(is);

         set.incrementFull(accepted, rejected, delivered, temperror,  permerror ,SmsTrOk ,SmsTrFailed,
              SmsTrBilled,UssdTrFromScOk, UssdTrFromScFailed ,0,0,UssdTrFromSmeFailed,UssdTrFromSmeBilled );
     }

     private void scanErrors(ExtendedCountersSet set, InputStream is) throws IOException {
         int counter = (int) readUInt32(is);
         while (counter-- > 0) {
             int errcode = (int) readUInt32(is);
             int count = (int) readUInt32(is);
             set.incError(errcode, count);
         }
     }

     private void scanSmes(HashMap map, InputStream is) throws IOException {
         int counter = (int) readUInt32(is);
         while (counter-- > 0) {
             int sme_id_len = readUInt8(is);
             String smeId = readString(is, sme_id_len);
             int providerId = (int)readUInt32(is);
             SmeIdCountersSet set = (SmeIdCountersSet) map.get(smeId);
             if (set == null) {
                 set = new SmeIdCountersSet(smeId);
                 map.put(smeId, set);
             }
             set.setProviderId(providerId);
             scanSmeCounters(set, is);
             scanErrors(set, is);
         }
     }

     private void scanRoutes(HashMap map, InputStream is) throws IOException {
         int counter = (int) readUInt32(is);
         while (counter-- > 0) {
             int route_id_len = readUInt8(is);
             String routeId = readString(is, route_id_len);

           //  long providerId = readInt64(is);
           //  long categoryId = readInt64(is);

             RouteIdCountersSet set = (RouteIdCountersSet) map.get(routeId);
             if (set == null) {
                 set = new RouteIdCountersSet(routeId);
                 map.put(routeId, set);
             }
            // set.setProviderId(providerId);
            // set.setCategoryId(categoryId);

             scanCounters(set, is);
             scanErrors(set, is);
         }
     }

     private static int readUInt8(InputStream is) throws IOException {
        int b = is.read();
        if (b == -1) throw new EOFException();
        return b;
    }
    private static int readUInt16(InputStream is) throws IOException {
        return (readUInt8(is) << 8 | readUInt8(is));
    }
    private static long readUInt32(InputStream is) throws IOException {
        return ((long) readUInt8(is) << 24) | ((long) readUInt8(is) << 16) |
                ((long) readUInt8(is) << 8) | ((long) readUInt8(is));
    }
    private static long readInt64(InputStream is) throws IOException {
        return (readUInt32(is) << 32) | readUInt32(is);
    }
    private static String readString(InputStream is, int size) throws IOException {
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
/*
  private void bindWherePart(PreparedStatement stmt, StatQuery query, boolean provider)
          throws SQLException
  {
    provider = (provider && (query.getProviderId() != StatQuery.ALL_PROVIDERS));
    int pos=1;
    if (query.isFromDateEnabled())
      stmt.setInt (pos++, calculatePeriod(query.getFromDate()));
    if (query.isTillDateEnabled())
      stmt.setInt (pos++, calculatePeriod(query.getTillDate()));
    if (provider)
      stmt.setLong(pos++, query.getProviderId());
  }
  private String prepareWherePart(StatQuery query, boolean provider)
  {
    provider = (provider && (query.getProviderId() != StatQuery.ALL_PROVIDERS));
    String str = (query.isFromDateEnabled() ||
            query.isTillDateEnabled() || provider) ? " WHERE ":"";
    if (query.isFromDateEnabled()) {
      str += " period >= ? ";
      if (query.isTillDateEnabled() || provider) str += " AND ";
    }
    if (query.isTillDateEnabled()) {
      str += " period <= ? ";
      if (provider) str += " AND ";
    }
    if (provider) {
      str += " providerid=? ";
    }
    return str;
  }

  private String prepareTotalQuery(StatQuery query) {
    return TOTAL_QUERY+prepareWherePart(query, true)+"GROUP BY period ORDER BY period ASC";
  }
  private String prepareSmeQuery(StatQuery query) {
    return SME_QUERY+prepareWherePart(query, true)+"GROUP BY systemid";
  }
  private String prepareRouteQuery(StatQuery query) {
    return ROUTE_QUERY+prepareWherePart(query, true)+"GROUP BY routeid";
  }
  private String prepareSmeStateQuery(StatQuery query) {
    return SME_STATE_QUERY+prepareWherePart(query, false)+"GROUP BY systemid, errcode";
  }
  private String prepareRouteStateQuery(StatQuery query) {
    return ROUTE_STATE_QUERY+prepareWherePart(query, false)+"GROUP BY routeid, errcode";
  }

  private PreparedStatement getQuery(Connection connection, StatQuery query, String sql, boolean provider)
          throws SQLException
  {
    PreparedStatement stmt = connection.prepareStatement(sql);
    bindWherePart(stmt, query, provider);
    return stmt;
  }

  private void processSmeQuery(Connection connection, StatQuery query)
          throws SQLException, Exception
  {
    HashMap countersForSme = new HashMap();
    PreparedStatement stmt = null;
    ResultSet rs = null;

    try
    {
      int oldPeriod = 0;
      DateCountersSet dateCounters = null;

      stmt = getQuery(connection, query, prepareTotalQuery(query), true);
      rs = stmt.executeQuery();
      while (rs.next())
      {
        int newPeriod = rs.getInt(1);
        int hour = calculateHour(newPeriod);
        HourCountersSet hourCounters = new HourCountersSet(
                rs.getLong(2), rs.getLong(3), rs.getLong(4), rs.getLong(5), rs.getLong(6),
                rs.getLong(7), rs.getLong(8), rs.getLong(9), rs.getLong(10), rs.getLong(11),
                rs.getLong(12), rs.getLong(13), rs.getLong(14), rs.getLong(15), hour);
        if (dateCounters == null) { // on first iteration
          Date date = calculateDate(newPeriod);
          dateCounters = new DateCountersSet(date);
        }
        else if (needChangeDate(oldPeriod, newPeriod)) { // date is unchanged
          stat.addDateStat(dateCounters);
          Date date = calculateDate(newPeriod);
          dateCounters = new DateCountersSet(date);
        }
        dateCounters.addHourStat(hourCounters);
        oldPeriod = newPeriod;
      }
      if (dateCounters != null) stat.addDateStat(dateCounters);
      rs.close(); rs = null; stmt.close(); stmt = null;

      stmt = getQuery(connection, query, prepareSmeQuery(query), true);
      rs = stmt.executeQuery();
      while (rs.next()) {
        String smeId = rs.getString(1);
        if (rs.wasNull() || smeId == null) continue;
        countersForSme.put(smeId,
                new SmeIdCountersSet(rs.getLong(2), rs.getLong(3), rs.getLong(4), rs.getLong(5), rs.getLong(6),
                        rs.getLong(7), rs.getLong(8), rs.getLong(9), rs.getLong(10), rs.getLong(11),
                        rs.getLong(12), rs.getLong(13), rs.getLong(14), rs.getLong(15), smeId));
      }
      rs.close(); rs = null; stmt.close(); stmt = null;

      stmt = getQuery(connection, query, prepareSmeStateQuery(query), false);
      rs = stmt.executeQuery();
      while (rs.next()) {
        String smeId = rs.getString(1);
        if (rs.wasNull() || smeId == null) continue;
        Object obj = countersForSme.get(smeId);
        if (obj != null && obj instanceof SmeIdCountersSet)
          ((SmeIdCountersSet)obj).addError(new ErrorCounterSet(rs.getInt(2), rs.getLong(3)));
      }

    } catch (SQLException ex) {
      throw ex;
    } finally {
      if (rs != null) rs.close();
      if (stmt != null) stmt.close();
    }

    Collection counters = countersForSme.values();
    if (counters != null && counters.size() > 0) stat.addSmeIdCollection(counters);
  }

  private void processRouteQuery(Connection connection, StatQuery query)
          throws SQLException
  {
    HashMap countersForRoute = new HashMap();
    PreparedStatement stmt = null;
    ResultSet rs = null;

    try
    {
      stmt = getQuery(connection, query, prepareRouteQuery(query), true);;
      rs = stmt.executeQuery();
      while (rs.next()) {
        String routeId = rs.getString(1);
        if (rs.wasNull() || routeId == null) continue;
        countersForRoute.put(routeId,
                new RouteIdCountersSet(rs.getLong(2), rs.getLong(3), rs.getLong(4), rs.getLong(5), rs.getLong(6),
                        routeId));
      }
      rs.close(); rs = null; stmt.close(); stmt = null;

      stmt = getQuery(connection, query, prepareRouteStateQuery(query), false);
      rs = stmt.executeQuery();
      while (rs.next()) {
        String routeId = rs.getString(1);
        if (rs.wasNull() || routeId == null) continue;
        Object obj = countersForRoute.get(routeId);
        if (obj != null && obj instanceof RouteIdCountersSet)
          ((RouteIdCountersSet)obj).addError(new ErrorCounterSet(rs.getInt(2), rs.getLong(3)));
      }

    } catch (SQLException ex) {
      throw ex;
    } finally {
      if (rs != null) rs.close();
      if (stmt != null) stmt.close();
    }

    Collection counters = countersForRoute.values();
    if (counters != null && counters.size() > 0) stat.addRouteIdCollection(counters);
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
 */
}
