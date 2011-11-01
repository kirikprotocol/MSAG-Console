package ru.novosoft.smsc.admin.stat;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.DBExportSettings;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.IOUtils;

import java.io.*;
import java.sql.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.Date;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * author: Aleksandr Khalitov
 */
public class SmscStatProvider {

  private static final Logger logger = Logger.getLogger(SmscStatProvider.class);

  private final static String DATE_DIR_FORMAT = "yyyy-MM";
  private final static String DATE_DAY_FORMAT = "yyyy-MM-dd HH:mm";
  private final static String DATE_DIR_FILE_FORMAT = DATE_DIR_FORMAT + File.separatorChar + "dd";
  private final static String DATE_FILE_EXTENSION = ".rts";

  private final SmscStatContext context;

  private final TimeZone defTimeZone = TimeZone.getDefault();
  private final TimeZone gmtTimeZone = TimeZone.getTimeZone("GMT");

  private final DBExportSettings defExportSettings;

  public SmscStatProvider(SmscStatContext context, DBExportSettings defExportSettings) {
    this.context = context;
    this.defExportSettings = defExportSettings;
  }

  public DBExportSettings getDefExportSettings() {
    return defExportSettings == null ? null : new DBExportSettings(defExportSettings);
  }

  private void readCounters(CountersSet set, InputStream is) throws IOException {
    int accepted = (int) IOUtils.readUInt32(is);
    int rejected = (int) IOUtils.readUInt32(is);
    int delivered = (int) IOUtils.readUInt32(is);
    int failed = (int) IOUtils.readUInt32(is);
    int rescheduled = (int) IOUtils.readUInt32(is);
    int temporal = (int) IOUtils.readUInt32(is);
    int peak_i = (int) IOUtils.readUInt32(is);
    int peak_o = (int) IOUtils.readUInt32(is);
    set.increment(accepted, rejected, delivered, failed, rescheduled,
        temporal, peak_i, peak_o);
  }

  private void readErrors(ExtendedCountersSet set, InputStream is) throws IOException {
    int counter = (int) IOUtils.readUInt32(is);
    while (counter-- > 0) {
      int errcode = (int) IOUtils.readUInt32(is);
      int count = (int) IOUtils.readUInt32(is);
      set.incError(errcode, count);
    }
  }

  private void readSmes(Map<String, SmeIdCountersSet> map, InputStream is) throws IOException {
    int counter = (int) IOUtils.readUInt32(is);
    while (counter-- > 0) {
      int sme_id_len = IOUtils.readUInt8(is);
      String smeId = IOUtils.readString(is, sme_id_len);
      SmeIdCountersSet set = map.get(smeId);
      if (set == null) {
        set = new SmeIdCountersSet(smeId);
        map.put(smeId, set);
      }
      readCounters(set, is);
      readErrors(set, is);
    }
  }

  private void readRoutes(Map<String, RouteIdCountersSet> map, InputStream is) throws IOException {
    int counter = (int) IOUtils.readUInt32(is);
    while (counter-- > 0) {
      int route_id_len = IOUtils.readUInt8(is);
      String routeId = IOUtils.readString(is, route_id_len);

      long providerId = IOUtils.readInt64(is);
      long categoryId = IOUtils.readInt64(is);

      RouteIdCountersSet set = map.get(routeId);
      if (set == null) {
        set = new RouteIdCountersSet(routeId);
        map.put(routeId, set);
      }
      set.setProviderId(providerId);
      set.setCategoryId(categoryId);

      readCounters(set, is);
      readErrors(set, is);
    }
  }

  private Date roundByDay(Date date) {
    if (date == null) {
      return null;
    }
    Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.HOUR_OF_DAY, 0);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    return c.getTime();
  }

  private Date roundByMonth(Date date) {
    if (date == null) {
      return null;
    }
    Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.DAY_OF_MONTH, 1);
    c.set(Calendar.HOUR_OF_DAY, 0);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    return c.getTime();
  }


  private void getStatFiles(File statPath, Collection<FileWithDate> result, Date fromDate, Date tillDate) {

    Date dirFromDate = roundByMonth(fromDate);
    Date dirTillDate = roundByMonth(tillDate);

    File[] dirs = context.getFileSystem().listFiles(statPath);
    if (dirs == null || dirs.length == 0) {
      return;
    }

    Date fileFromDate = roundByDay(fromDate);
    Date fileTillDate = roundByDay(tillDate);


    SimpleDateFormat dateDirFormat = new SimpleDateFormat(DATE_DIR_FORMAT);
    SimpleDateFormat dateDirFileFormat = new SimpleDateFormat(DATE_DIR_FILE_FORMAT);

    for (File dir : dirs) {

      Date dirDate;
      try {
        dirDate = dateDirFormat.parse(dir.getName());
      } catch (ParseException ignored) {
        continue;
      }

      if (dirFromDate != null && dirDate.compareTo(dirFromDate) < 0) continue;
      if (dirTillDate != null && dirDate.compareTo(dirTillDate) > 0) continue;


      File[] dirFiles = getStatsFiles(dir);

      if (dirFiles == null || dirFiles.length == 0) continue;

      for (File dirFile : dirFiles) {
        String fileName = dirFile.getName();

        Date fileDate;
        try {
          fileDate = dateDirFileFormat.parse(dir.getName() + File.separatorChar + fileName);
        } catch (ParseException exc) {
          continue;
        }

        if (fileFromDate != null && fileDate.compareTo(fileFromDate) < 0) continue;
        if (fileTillDate != null && fileDate.compareTo(fileTillDate) > 0) continue;

        result.add(new FileWithDate(dirFile, fileDate));
      }
    }

  }

  private File[] getStatsFiles(File dir) {
    return context.getFileSystem().listFiles(dir, new FileFilter() {
      public boolean accept(File pathname) {
        return !context.getFileSystem().isDirectory(pathname) && pathname.getName().toLowerCase().endsWith(DATE_FILE_EXTENSION);
      }
    });
  }

  private TreeSet<FileWithDate> getStatFiles(Date fromQueryDate, Date tillQueryDate) throws AdminException {
    TreeSet<FileWithDate> result = new TreeSet<FileWithDate>();

    Date fromDate = fromQueryDate == null ? null : Functions.convertTime(fromQueryDate, defTimeZone, gmtTimeZone);
    Date tillDate = tillQueryDate == null ? null : Functions.convertTime(tillQueryDate, defTimeZone, gmtTimeZone);

    for (File statPath : context.getStatDirs()) {
      getStatFiles(statPath, result, fromDate, tillDate);
    }
    return result;
  }


  private static void checkFileHeader(InputStream input, String filename) throws IOException, StatException {
    String fileStamp = IOUtils.readString(input, 9);
    if (fileStamp == null || !fileStamp.equals("SMSC.STAT")) {
      throw new StatException("unsupported_file_format", filename);
    }
    IOUtils.readUInt16(input); // read version for support reasons
  }

  private boolean processFile(FileWithDate fwd, Date fromQueryDate, Date tillQueryDate, Visitor visitor) throws AdminException, VisitorException {

    SimpleDateFormat dateDayFormat = new SimpleDateFormat(DATE_DAY_FORMAT);

    boolean result = true;

    Date fileDate = fwd.date; // GMT
    File path = fwd.file;
    if (logger.isDebugEnabled()) {
      logger.debug("Parsing file: " + dateDayFormat.format(fileDate) + " GMT");
    }
    InputStream input = null;
    try {
      input = new BufferedInputStream(context.getFileSystem().getInputStream(path));
      checkFileHeader(input, path.getAbsolutePath());
      CountersSet lastHourCounter = new CountersSet();
      Map<String, SmeIdCountersSet> countersForSme = new HashMap<String, SmeIdCountersSet>();
      Map<String, RouteIdCountersSet> countersForRoute = new HashMap<String, RouteIdCountersSet>();
      ExtendedCountersSet errors = new ExtendedCountersSet();
      Date lastDate = null;
      Date curDate;
      int prevHour = -1;
      byte buffer[] = new byte[512 * 1024];
      boolean haveValues = false;
      int recordNum = 0;

      while (true) // iterate file records (by minutes)
      {
        try {
          recordNum++;
          int rs1 = (int) IOUtils.readUInt32(input);
          if (buffer.length < rs1) buffer = new byte[rs1];
          readBuffer(input, buffer, rs1);
          int rs2 = (int) IOUtils.readUInt32(input);
          if (rs1 != rs2) {
            throw new StatException("unsupported_file_format", path.getAbsolutePath());
          }

          ByteArrayInputStream is = new ByteArrayInputStream(buffer, 0, rs1);
          try {
            int hour = IOUtils.readUInt8(is);
            int min = IOUtils.readUInt8(is);
            Calendar calendar = Calendar.getInstance();
            calendar.setTime(fileDate);
            calendar.set(Calendar.HOUR, hour);
            calendar.set(Calendar.MINUTE, min);
            curDate = Functions.convertTime(calendar.getTime(), gmtTimeZone, defTimeZone);

            if (fromQueryDate != null && curDate.getTime() < fromQueryDate.getTime()) {
              //logger.debug("Hour: "+hour+" skipped");
              continue;
            }

            if (prevHour == -1) prevHour = hour;
            if (lastDate == null) lastDate = curDate;

            if (hour != prevHour && haveValues) { // switch to new hour
              if (logger.isDebugEnabled()) {
                logger.debug("New hour: " + hour + ", dump stat for: " + dateDayFormat.format(lastDate) + " GMT");
              }
              visitor.visit(lastDate, lastHourCounter, errors, countersForSme.values(), countersForRoute.values());
              haveValues = false;
              lastDate = curDate;
              prevHour = hour;
              lastHourCounter = new CountersSet();
              errors = new ExtendedCountersSet();
              countersForSme.clear();
              countersForRoute.clear();
            }

            if (tillQueryDate != null && curDate.getTime() >= tillQueryDate.getTime()) {
              result = false;
              break; // finish work
            }


            haveValues = true; // read and increase counters
            readCounters(lastHourCounter, is);
            readErrors(errors, is);
            readSmes(countersForSme, is);
            readRoutes(countersForRoute, is);
          } catch (EOFException exc) {
            logger.warn("Incomplete record #" + recordNum + " in '" + path.getAbsolutePath() + '\'');
          }
        } catch (EOFException exc) {
          break;
        }
      }
      if (haveValues) {
        if (logger.isDebugEnabled()) {
          logger.debug("Last dump stat for: " + dateDayFormat.format(lastDate) + " GMT");
        }
        visitor.visit(lastDate, lastHourCounter, errors, countersForSme.values(), countersForRoute.values());
      }

    } catch (IOException e) {
      logger.error(e, e);
      throw new StatException("internal_exception");
    } finally {
      try {
        if (input != null) input.close();
      } catch (Throwable th) {
        th.printStackTrace();
      }
    }
    return result;
  }

  public Statistics getStatistics(SmscStatFilter filter) throws AdminException {
    return getStatistics(filter, null);
  }


  public Statistics getStatistics(SmscStatFilter filter, SmscStatLoadListener loadListener) throws AdminException {

    SimpleDateFormat dateDayFormat = new SimpleDateFormat(DATE_DAY_FORMAT);
    SimpleDateFormat dateDayLocalFormat = new SimpleDateFormat(DATE_DAY_FORMAT);


    Date fromQueryDate = filter == null ? null : filter.getFrom();
    Date tillQueryDate = filter == null ? null : filter.getTill();

    if (logger.isDebugEnabled()) {
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
      logger.debug("Query stat" + fromDate + tillDate);
    }

    final Statistics stat = new Statistics();
    Set<FileWithDate> selectedFiles = getStatFiles(fromQueryDate, tillQueryDate);
    if (selectedFiles.size() <= 0) return stat;

    if (loadListener != null) {
      loadListener.setTotal(selectedFiles.size() + 1);
    }

    final Map<Date, CountersSet> statByHours = new TreeMap<Date, CountersSet>();
    final Map<String, SmeIdCountersSet> countersForSme = new HashMap<String, SmeIdCountersSet>();
    final Map<String, RouteIdCountersSet> countersForRoute = new HashMap<String, RouteIdCountersSet>();

    long tm = System.currentTimeMillis();
    for (FileWithDate fwd : selectedFiles) {
      try {
        processFile(fwd, fromQueryDate, tillQueryDate, new Visitor() {
          public void visit(Date date, CountersSet lastHourSet, ExtendedCountersSet errorsSet, Collection<SmeIdCountersSet> _countersForSme, Collection<RouteIdCountersSet> _countersForRoute) {
            CountersSet hcs = statByHours.get(date);
            if (hcs == null) {
              statByHours.put(date, lastHourSet);
            } else {
              hcs.increment(lastHourSet);
            }
            for (ErrorCounterSet e : errorsSet.getErrors()) {
              stat.incError(e.getErrcode(), e.getCounter());
            }
            for (SmeIdCountersSet smeIdCountersSet : _countersForSme) {
              if (!countersForSme.containsKey(smeIdCountersSet.getSmeid())) {
                countersForSme.put(smeIdCountersSet.getSmeid(), smeIdCountersSet);
              } else {
                SmeIdCountersSet old = countersForSme.get(smeIdCountersSet.getSmeid());
                for (ErrorCounterSet e : smeIdCountersSet.getErrors()) {
                  old.incError(e.getErrcode(), e.getCounter());
                }
                old.increment(smeIdCountersSet);
              }
            }
            for (RouteIdCountersSet routeIdCountersSet : _countersForRoute) {
              if (!countersForRoute.containsKey(routeIdCountersSet.getRouteid())) {
                countersForRoute.put(routeIdCountersSet.getRouteid(), routeIdCountersSet);
              } else {
                RouteIdCountersSet old = countersForRoute.get(routeIdCountersSet.getRouteid());
                for (ErrorCounterSet e : routeIdCountersSet.getErrors()) {
                  old.incError(e.getErrcode(), e.getCounter());
                }
                old.increment(routeIdCountersSet);
              }
            }
          }
        });
      } catch (VisitorException e) {
        logger.error(e, e);
      }
      if (loadListener != null) {
        loadListener.incrementProgress();
      }
    }
    if (logger.isDebugEnabled()) {
      logger.debug("End scanning statistics, time spent: " + ((System.currentTimeMillis() - tm) / 1000) + " sec");
    }

    Calendar localCalendar = Calendar.getInstance();
    DateCountersSet dateCounters = null;
    Date lastDate = null;

    for (Map.Entry<Date, CountersSet> e : statByHours.entrySet()) {
      Date hourDate = e.getKey();
      CountersSet hourCounter = e.getValue();

      localCalendar.setTime(hourDate);
      int hour = localCalendar.get(Calendar.HOUR_OF_DAY);

      if (lastDate == null || !roundByDay(hourDate).equals(lastDate)) {
        localCalendar.set(Calendar.HOUR_OF_DAY, 0);
        localCalendar.set(Calendar.MINUTE, 0);
        localCalendar.set(Calendar.SECOND, 0);
        localCalendar.set(Calendar.MILLISECOND, 0);
        lastDate = localCalendar.getTime();
        if (dateCounters != null) {
          stat.addDateStat(dateCounters);
        }
        dateCounters = new DateCountersSet(lastDate);
      }

      HourCountersSet set = new HourCountersSet(hour);
      set.increment(hourCounter);
      dateCounters.addHourStat(set);
    }
    if (dateCounters != null) {
      stat.addDateStat(dateCounters);
    }

    Collection<SmeIdCountersSet> countersSme = countersForSme.values();
    if (countersSme != null) stat.addSmeIdCollection(countersSme);
    Collection<RouteIdCountersSet> countersRoute = countersForRoute.values();
    if (countersRoute != null) stat.addRouteIdCollection(countersRoute);

    if (loadListener != null) {
      loadListener.incrementProgress();
    }

    return stat;
  }

  private final static String WHERE_OP_SQL = " WHERE";
  private final static String DELETE_OP_SQL = "DELETE FROM ";
  private final static String INSERT_OP_SQL = "INSERT INTO ";

  private final static String VALUES_SMS_SQL =
      " (period, accepted, rejected, delivered, failed, rescheduled, temporal, peak_i, peak_o)" +
          " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
  private final static String VALUES_SMS_ERR_SQL =
      " (period, errcode, counter) VALUES (?, ?, ?)";
  private final static String VALUES_SME_SQL =
      " (period, systemid, accepted, rejected, delivered, failed, rescheduled, temporal, peak_i, peak_o)" +
          " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
  private final static String VALUES_SME_ERR_SQL =
      " (period, systemid, errcode, counter) VALUES (?, ?, ?, ?)";
  private final static String VALUES_ROUTE_SQL =
      " (period, routeid, accepted, rejected, delivered, failed, rescheduled, temporal, peak_i, peak_o)" +
          " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
  private final static String VALUES_ROUTE_ERR_SQL =
      " (period, routeid, errcode, counter) VALUES (?, ?, ?, ?)";


  private static final String mysqlDriver = "com.mysql.jdbc.Driver";
  private static final String oracleDriver = "oracle.jdbc.driver.OracleDriver";

  private Connection getConnection(DBExportSettings export) throws SQLException, ClassNotFoundException {
    switch (export.getDbType()) {
      case MYSQL:
        Class.forName(mysqlDriver);
        break;
      case ORACLE:
        Class.forName(oracleDriver);
    }
    Connection c = DriverManager.getConnection(export.getSource(), export.getUser(), export.getPass());
    c.setAutoCommit(false);
    return c;
  }

  private String prepareWherePart(long from, long till) {
    if (from < -1 && till < -1) return "";
    String result = WHERE_OP_SQL;
    if (from > 0) {
      result += (" period >= " + from);
    }
    if (till > 0) {
      if (from > 0) result += " AND";
      result += (" period < " + till);
    }
    return result;
  }

  private long calculatePeriod(Date date) {
    SimpleDateFormat datePeriodLocalFormat = new SimpleDateFormat("yyyyMMddHHmm");
    String str = datePeriodLocalFormat.format(date);
    return Long.parseLong(str);
  }

  private void setValues(PreparedStatement stmt, long period, String id, CountersSet set) throws SQLException {
    int pos = 1;
    stmt.setLong(pos++, period);
    if (id != null) stmt.setString(pos++, id);
    stmt.setLong(pos++, set.getAccepted());
    stmt.setLong(pos++, set.getRejected());
    stmt.setLong(pos++, set.getDelivered());
    stmt.setLong(pos++, set.getFailed());
    stmt.setLong(pos++, set.getRescheduled());
    stmt.setLong(pos++, set.getTemporal());
    stmt.setLong(pos++, set.getPeak_i());
    stmt.setLong(pos, set.getPeak_o());
  }

  private void setError(PreparedStatement stmt, long period, String id, ErrorCounterSet err) throws SQLException {
    int pos = 1;
    stmt.setLong(pos++, period);
    if (id != null) stmt.setString(pos++, id);
    stmt.setInt(pos++, err.getErrcode());
    stmt.setLong(pos, err.getCounter());
  }

  private void closeConnection(Connection connection) {
    if (connection != null) {
      try {
        connection.close();
      } catch (Exception ignored) {
      }
    }
  }

  private void rollbackConnection(Connection connection) {
    if (connection != null) {
      try {
        connection.rollback();
      } catch (Exception ignored) {
      }
    }
  }

  private void closeStatement(Statement stmt) {
    if (stmt != null) {
      try {
        stmt.close();
      } catch (Exception ignored) {
      }
    }
  }

  private void dumpTotalCounters(PreparedStatement stmt, PreparedStatement errStmt,
                                 ExtendedCountersSet set, long period,
                                 ExportResults results) throws SQLException {
    setValues(stmt, period, null, set);
    stmt.executeUpdate();
    results.total.records++;
    for (ErrorCounterSet err : set.getErrors()) {
      if (err == null) continue;
      setError(errStmt, period, null, err);
      errStmt.executeUpdate();
      results.total.errors++;
    }
  }

  private void dumpSmeCounters(PreparedStatement stmt, PreparedStatement errStmt,
                               Collection<SmeIdCountersSet> map, long period,
                               ExportResults results) throws SQLException {
    for (SmeIdCountersSet sme : map) {
      if (sme == null) continue;
      setValues(stmt, period, sme.getSmeid(), sme);
      stmt.executeUpdate();
      results.smes.records++;
      for (ErrorCounterSet err : sme.getErrors()) {
        if (err == null) continue;
        setError(errStmt, period, sme.getSmeid(), err);
        errStmt.executeUpdate();
        results.smes.errors++;
      }
    }
  }

  private void dumpRouteCounters(PreparedStatement stmt, PreparedStatement errStmt,
                                 Collection<RouteIdCountersSet> map, long period,
                                 ExportResults results) throws SQLException {
    for (RouteIdCountersSet route : map) {
      if (route == null) continue;
      setValues(stmt, period, route.getRouteid(), route);
      stmt.executeUpdate();
      results.routes.records++;
      for (ErrorCounterSet err : route.getErrors()) {
        if (err == null) continue;
        setError(errStmt, period, route.getRouteid(), err);
        errStmt.executeUpdate();
        results.routes.errors++;
      }
    }
  }

  private void cleanTable(Connection connection, long from, long to, String totalSmsTable,
                          String totalErrTable, String smeSmsTable, String smeErrTable, String routeSmsTable, String routeErrTable) throws SQLException {
    Statement stmt = null;
    try {
      stmt = connection.createStatement();
      final String wherePart = prepareWherePart(from, to);
      stmt.executeUpdate(DELETE_OP_SQL + totalSmsTable + wherePart);
      stmt.executeUpdate(DELETE_OP_SQL + totalErrTable + wherePart);
      stmt.executeUpdate(DELETE_OP_SQL + smeSmsTable + wherePart);
      stmt.executeUpdate(DELETE_OP_SQL + smeErrTable + wherePart);
      stmt.executeUpdate(DELETE_OP_SQL + routeSmsTable + wherePart);
      stmt.executeUpdate(DELETE_OP_SQL + routeErrTable + wherePart);
      connection.commit(); // commit old stat delete

      logger.debug("Old statistics data deleted");
    } finally {
      if (stmt != null) {
        try {
          stmt.close();
        } catch (Exception ignored) {
        }
      }
    }
  }


  public ExportResults exportStatistics(SmscStatFilter filter, SmscStatLoadListener loadListener) throws AdminException {
    return exportStatistics(filter, defExportSettings, loadListener);
  }

  private final Lock lock = new ReentrantLock();

  public ExportResults exportStatistics(SmscStatFilter filter, DBExportSettings export, SmscStatLoadListener loadListener) throws AdminException {
    final ExportResults results = new ExportResults();
    final String tablesPrefix = export.getPrefix();
    final String totalSmsTable = tablesPrefix + "_sms";
    final String totalErrTable = tablesPrefix + "_state";
    final String smeSmsTable = tablesPrefix + "_sme";
    final String smeErrTable = tablesPrefix + "_sme_state";
    final String routeSmsTable = tablesPrefix + "_route";
    final String routeErrTable = tablesPrefix + "_route_state";

    long fromPeriod = filter.getFrom() != null ? calculatePeriod(filter.getFrom()) : -1;
    long tillPeriod = filter.getTill() != null ? calculatePeriod(filter.getTill()) : -1;

    Connection connection = null;
    Statement stmt = null;
    PreparedStatement insertSms = null;
    PreparedStatement insertErr = null;
    PreparedStatement insertSmeSms = null;
    PreparedStatement insertSmeErr = null;
    PreparedStatement insertRouteSms = null;
    PreparedStatement insertRouteErr = null;
    try {

      try {
        connection = getConnection(export);
      } catch (Exception e) {
        throw new StatException("cant_connect");
      }
      try{
        lock.lock();
        if(logger.isDebugEnabled()) {
          logger.debug("Smsc stat locked");
        }
        try {
          cleanTable(connection, fromPeriod, tillPeriod, totalSmsTable, totalErrTable, smeSmsTable, smeErrTable, routeSmsTable, routeErrTable);
        } catch (Exception e) {
          throw new StatException("cant_clean");
        }

        try {
          insertSms = connection.prepareStatement(INSERT_OP_SQL + totalSmsTable + VALUES_SMS_SQL);
          insertErr = connection.prepareStatement(INSERT_OP_SQL + totalErrTable + VALUES_SMS_ERR_SQL);
          insertSmeSms = connection.prepareStatement(INSERT_OP_SQL + smeSmsTable + VALUES_SME_SQL);
          insertSmeErr = connection.prepareStatement(INSERT_OP_SQL + smeErrTable + VALUES_SME_ERR_SQL);
          insertRouteSms = connection.prepareStatement(INSERT_OP_SQL + routeSmsTable + VALUES_ROUTE_SQL);
          insertRouteErr = connection.prepareStatement(INSERT_OP_SQL + routeErrTable + VALUES_ROUTE_ERR_SQL);
        } catch (SQLException e) {
          throw new StatException("internal_error");
        }

        long tm = System.currentTimeMillis();

        Set<FileWithDate> selectedFiles = getStatFiles(filter.getFrom(), filter.getTill());
        if (selectedFiles.isEmpty()) return results;

        if (loadListener != null) {
          loadListener.setTotal(selectedFiles.size());
        }

        for (FileWithDate file : selectedFiles) {
          final Connection conn = connection;
          final PreparedStatement _insertSms = insertSms;
          final PreparedStatement _insertErr = insertErr;
          final PreparedStatement _insertSmeSms = insertSmeSms;
          final PreparedStatement _insertSmeErr = insertSmeErr;
          final PreparedStatement _insertRouteSms = insertRouteSms;
          final PreparedStatement _insertRouteErr = insertRouteErr;
          try {
            processFile(file, filter.getFrom(), filter.getTill(), new Visitor() {
              public void visit(Date lastDate, CountersSet lastHourSet, ExtendedCountersSet errorsSet,
                                Collection<SmeIdCountersSet> smeCounters, Collection<RouteIdCountersSet> routeCounters) throws VisitorException {

                errorsSet.increment(lastHourSet);

                try {
                  long period = calculatePeriod(lastDate); // dump counters to DB
                  dumpTotalCounters(_insertSms, _insertErr, errorsSet, period, results);
                  dumpSmeCounters(_insertSmeSms, _insertSmeErr, smeCounters, period, results);
                  dumpRouteCounters(_insertRouteSms, _insertRouteErr, routeCounters, period, results);
                  conn.commit();
                } catch (Exception e) {
                  throw new VisitorException(e);
                }

              }
            });
          } catch (VisitorException e) {
            throw new StatException("cant_insert");
          }
          if (loadListener != null) {
            loadListener.incrementProgress();
          }
        }
        logger.debug("End dumping statistics at: " + new Date() + " time spent: " +
            (System.currentTimeMillis() - tm) / 1000);
        return results;
      }finally {
        lock.unlock();
        if(logger.isDebugEnabled()) {
          logger.debug("Smsc stat unlocked");
        }
      }
    } catch (AdminException exc) {
      logger.error(exc, exc);
      rollbackConnection(connection);
      throw exc;
    } finally {
      closeStatement(stmt);
      closeStatement(insertSms);
      closeStatement(insertErr);
      closeStatement(insertSmeSms);
      closeStatement(insertSmeErr);
      closeStatement(insertRouteSms);
      closeStatement(insertRouteErr);
      closeConnection(connection);
    }
  }


  private static void readBuffer(InputStream is, byte buffer[], int size) throws IOException {
    int read = 0;
    while (read < size) {
      int result = is.read(buffer, read, size - read);
      if (result < 0) throw new EOFException("Failed to read " + size + " bytes, read failed at " + read);
      read += result;
    }
  }

  private static class FileWithDate implements Comparable<FileWithDate> {
    private final File file;
    private final Date date;  //GMT

    private FileWithDate(File file, Date date) {
      this.file = file;
      this.date = date;
    }

    public int compareTo(FileWithDate o) {
      int res = this.date.compareTo(o.date);
      return res != 0 ? res : file.compareTo(o.file);
    }

    @Override
    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;

      FileWithDate that = (FileWithDate) o;

      if (date != null ? !date.equals(that.date) : that.date != null) return false;
      if (file != null ? !file.equals(that.file) : that.file != null) return false;

      return true;
    }

    @Override
    public int hashCode() {
      int result = file != null ? file.hashCode() : 0;
      result = 31 * result + (date != null ? date.hashCode() : 0);
      return result;
    }
  }

  private static class VisitorException extends Exception {
    private VisitorException(Throwable cause) {
      super(cause);
    }
  }

  private static interface Visitor {
    public void visit(Date date, CountersSet lastHourSet, ExtendedCountersSet errorsSet, Collection<SmeIdCountersSet> countersForSme,
                      Collection<RouteIdCountersSet> countersForRoute) throws VisitorException;
  }


}
