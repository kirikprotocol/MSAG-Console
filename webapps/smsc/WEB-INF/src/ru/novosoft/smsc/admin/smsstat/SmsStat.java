package ru.novosoft.smsc.admin.smsstat;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.sibinco.util.conpool.ConnectionPool;

import java.io.*;
import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.sql.Statement;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.PreparedStatement;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 22.04.2005
 * Time: 15:32:13
 * To change this template use File | Settings | File Templates.
 */
public class SmsStat
 {
    org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(SmsStat.class);

    private final static String DATE_DIR_FORMAT = "yyyy-MM";
    private final static String DATE_DAY_FORMAT = "yyyy-MM-dd HH:mm";
    private final static String DATE_DIR_FILE_FORMAT = DATE_DIR_FORMAT + File.separatorChar + "dd";
    private final static String DATE_FILE_EXTENSION  = ".rts";
    private final static String DATE_PERIOD_FORMAT = "yyyyMMddHHmm";

    private Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
    private Calendar localCaledar = Calendar.getInstance(TimeZone.getDefault());
    private SimpleDateFormat dateDirFormat = new SimpleDateFormat(DATE_DIR_FORMAT);
    private SimpleDateFormat dateDirFileFormat = new SimpleDateFormat(DATE_DIR_FILE_FORMAT);
    private SimpleDateFormat dateDayFormat = new SimpleDateFormat(DATE_DAY_FORMAT);
    private SimpleDateFormat dateDayLocalFormat = new SimpleDateFormat(DATE_DAY_FORMAT);
    private SimpleDateFormat datePeriodLocalFormat = new SimpleDateFormat(DATE_PERIOD_FORMAT);

    private String statstorePath;
    private final static String PARAM_NAME_STAT_DIR = "MessageStore.statisticsDir";

    private Date fromQueryDate = null;
    private Date tillQueryDate = null;

    private ExportSettings defaultExportSettings = null;

    private static Object instanceLock = new Object();
    private static SmsStat instance = null;

    public static SmsStat getInstance(Config smscConfig, Config webConfig) throws AdminException
    {
        synchronized(instanceLock) {
            if (instance == null) instance = new SmsStat(smscConfig, webConfig);
            return instance;
        }
    }

    protected SmsStat(Config smscConfig, Config webConfig) throws AdminException
    {
        try {
            statstorePath = smscConfig.getString(PARAM_NAME_STAT_DIR);
            if (statstorePath == null || statstorePath.length() <= 0)
                throw new AdminException("store path is empty");
        } catch (Exception e) {
            throw new AdminException("Failed to obtain statistics dir. Details: " + e.getMessage());
        }
        try {
            final String section = "statsave_datasource";
            final String source = webConfig.getString(section + ".source");
            final String driver = webConfig.getString(section + ".driver");
            final String user   = webConfig.getString(section + ".user");
            final String pass   = webConfig.getString(section + ".pass");
            final String prefix = webConfig.getString(section + ".tables_prefix");
            defaultExportSettings = new ExportSettings(source, driver, user, pass, prefix);
        } catch (Exception e) {
            throw new AdminException("Failed to configure default export settings. Details: " + e.getMessage());
        }

        dateDirFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        dateDirFileFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        dateDayFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        dateDayLocalFormat.setTimeZone(TimeZone.getDefault());
        datePeriodLocalFormat.setTimeZone(TimeZone.getDefault());
    }

    private TreeMap getStatQueryDirs() throws AdminException
    {
        File statPath = new File(statstorePath);
        if (statPath == null || !statPath.isAbsolute()) {
            File smscConfFile = WebAppFolders.getSmscConfFolder();
            String smscDirFile = smscConfFile.getParent();
            statPath = new File(smscDirFile, statstorePath);
            logger.debug("Stat path: by smsc conf '"+statPath.getAbsolutePath()+"'");
        } else {
            logger.debug("Stat path: is absolute '"+statPath.getAbsolutePath()+"'");
        }

        String[] dirNames = statPath.list();
        if (dirNames == null || dirNames.length == 0)
            throw new AdminException("No stat directories at path '"+statPath.getAbsolutePath()+"'");

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

    private void initQueryPeriod(StatQuery query) {
        fromQueryDate = query.isFromDateEnabled() ? query.getFromDate() : null;
        tillQueryDate = query.isTillDateEnabled() ? query.getTillDate() : null;
    }

    private void scanCounters(CountersSet set, InputStream is) throws IOException {
        int accepted = (int) readUInt32(is);
        int rejected = (int) readUInt32(is);
        int delivered = (int) readUInt32(is);
        int failed = (int) readUInt32(is);
        int rescheduled = (int) readUInt32(is);
        int temporal = (int) readUInt32(is);
        int peak_i = (int) readUInt32(is);
        int peak_o = (int) readUInt32(is);
        set.increment(accepted, rejected, delivered, failed, rescheduled,
                      temporal, peak_i, peak_o);
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
            SmeIdCountersSet set = (SmeIdCountersSet) map.get(smeId);
            if (set == null) {
                set = new SmeIdCountersSet(smeId);
                map.put(smeId, set);
            }
            scanCounters(set, is);
            scanErrors(set, is);
        }
    }

    private void scanRoutes(HashMap map, InputStream is) throws IOException {
        int counter = (int) readUInt32(is);
        while (counter-- > 0) {
            int route_id_len = readUInt8(is);
            String routeId = readString(is, route_id_len);

            long providerId = readInt64(is);
            long categoryId = readInt64(is);

            RouteIdCountersSet set = (RouteIdCountersSet) map.get(routeId);
            if (set == null) {
                set = new RouteIdCountersSet(routeId);
                map.put(routeId, set);
            }
            set.setProviderId(providerId);
            set.setCategoryId(categoryId);

            scanCounters(set, is);
            scanErrors(set, is);
        }
    }

    private Object generalLock = new Object();

    public Statistics getStatistics(StatQuery query) throws AdminException
    {
        synchronized(generalLock) {
           return _getStatistics(query);
        }
    }
    public void exportStatistics(StatQuery query, ExportResults results,
                                 ExportSettings export) throws AdminException
    {
        synchronized(generalLock) {
           _exportStatistics(query, results, (export != null) ? export : defaultExportSettings);
        }
    }
    public void exportStatistics(StatQuery query, ExportResults results) throws AdminException
    {
        synchronized(generalLock) {
           _exportStatistics(query, results, defaultExportSettings);
        }
    }
    public ExportSettings getDefaultExportSettings() {
        return defaultExportSettings;
    }

    private Statistics _getStatistics(StatQuery query) throws AdminException
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

        TreeMap statByHours = new TreeMap(); // to add lastHourCounter to it
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
                String fileStamp = readString(input, 9);
                if (fileStamp == null || !fileStamp.equals("SMSC.STAT"))
                    throw new AdminException("unsupported header of file (support only SMSC.STAT file )");
                readUInt16(input); // read version for support reasons
                CountersSet lastHourCounter = new CountersSet();
                Date lastDate = null;
                Date curDate = null;
                int prevHour = -1;
                byte buffer[] = new byte[512 * 1024];
                boolean haveValues = false;
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
                                //logger.debug("Hour: "+hour+" skipped");
                                continue;
                            }

                            if (prevHour == -1) prevHour = hour;
                            if (lastDate == null) lastDate = curDate;

                            if (hour != prevHour && haveValues) { // switch to new hour
                                logger.debug("New hour: " + hour + ", dump stat for: " + dateDayFormat.format(lastDate) + " GMT");
                                statByHours.put(lastDate, lastHourCounter);
                                haveValues = false;
                                lastDate = curDate;
                                prevHour = hour;
                                lastHourCounter = new CountersSet();
                            }

                            if (tillQueryDate != null && curDate.getTime() >= tillQueryDate.getTime()) {
                                finished = true;
                                break; // finish work
                            }

                            haveValues = true; // read and increase counters
                            scanCounters(lastHourCounter, is);
                            scanErrors(stat, is);
                            scanSmes(countersForSme, is);
                            scanRoutes(countersForRoute, is);
                        } catch (EOFException exc) {
                            logger.warn("Incomplete record #"+recordNum+" in "+path+"");
                        }
                    } catch (EOFException exc) {
                        break;
                    }
                }
                if (haveValues) {
                    logger.debug("Last dump stat for: " + dateDayFormat.format(lastDate) + " GMT");
                    statByHours.put(lastDate, lastHourCounter);
                }

            } catch (IOException e) {
                throw new AdminException(e.getMessage());
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

        Collection countersSme = countersForSme.values();
        if (countersSme != null) stat.addSmeIdCollection(countersSme);
        Collection countersRoute = countersForRoute.values();
        if (countersRoute != null) stat.addRouteIdCollection(countersRoute);

        return stat;
    }

    private final static String WHERE_OP_SQL  = " WHERE";
    private final static String DELETE_OP_SQL = "DELETE FROM ";
    private final static String INSERT_OP_SQL = "INSERT INTO ";
    
    private final static String VALUES_SMS_SQL = 
        " (period, accepted, rejected, delivered, failed, rescheduled, temporal, peak_i, peak_o)"+
        " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
    private final static String VALUES_SMS_ERR_SQL =
        " (period, errcode, counter) VALUES (?, ?, ?)";
    private final static String VALUES_SME_SQL =
        " (period, systemid, accepted, rejected, delivered, failed, rescheduled, temporal, peak_i, peak_o)"+
        " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    private final static String VALUES_SME_ERR_SQL =
        " (period, systemid, errcode, counter) VALUES (?, ?, ?, ?)";
    private final static String VALUES_ROUTE_SQL =
        " (period, routeid, accepted, rejected, delivered, failed, rescheduled, temporal, peak_i, peak_o)"+
        " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    private final static String VALUES_ROUTE_ERR_SQL =
        " (period, routeid, errcode, counter) VALUES (?, ?, ?, ?)";

    private ConnectionPool createDataSource(ExportSettings export) throws SQLException
    {
      Properties props = new Properties();
      props.setProperty("jdbc.source", export.getSource());
      props.setProperty("jdbc.driver", export.getDriver());
      props.setProperty("jdbc.user", export.getUser());
      props.setProperty("jdbc.pass", export.getPassword());
      return new ConnectionPool(props);
    }

    private String prepareWherePart(long from, long till)
    {
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
    private long calculatePeriod(Date date) throws AdminException
    {
        try {
          String str = datePeriodLocalFormat.format(date);
          return Long.parseLong(str);
        } catch (Exception exc) {
            logger.error(exc);
            throw new AdminException("Invalid period format. Details: "+exc.getMessage());
        }
    }

    private void setValues(PreparedStatement stmt, long period, String id, CountersSet set) throws SQLException
    {
        int pos = 1;
        stmt.setLong(pos++, period);
        if (id != null) stmt.setString(pos++, id);
        stmt.setLong(pos++, set.accepted);
        stmt.setLong(pos++, set.rejected);
        stmt.setLong(pos++, set.delivered);
        stmt.setLong(pos++, set.failed);
        stmt.setLong(pos++, set.rescheduled);
        stmt.setLong(pos++, set.temporal);
        stmt.setLong(pos++, set.peak_i);
        stmt.setLong(pos++, set.peak_o);
    }
    private void setError(PreparedStatement stmt, long period, String id, ErrorCounterSet err) throws SQLException
    {
        int pos = 1;
        stmt.setLong(pos++, period);
        if (id != null) stmt.setString(pos++, id);
        stmt.setInt(pos++, err.errcode);
        stmt.setLong(pos++, err.counter);
    }

    private void shutdownPool(ConnectionPool pool)
    {
        try { if (pool != null) pool.shutdown(); }
        catch(Throwable th) { logger.error("Failed to close data source", th); }
    }
    private void closeConnection(Connection connection)
    {
        try { if (connection != null) connection.close(); }
        catch(Throwable th) { logger.error("Failed to close connection", th); }
    }
    private void rollbackConnection(Connection connection)
    {
        try { if (connection != null) connection.rollback(); }
        catch(Throwable th) { logger.error("Failed to rollback on connection", th); }
    }
    private void closeStatement(Statement stmt)
    {
        try { if (stmt != null) stmt.close(); }
        catch(Throwable th) { logger.error("Failed to close statement", th); }
    }
    
    private void dumpTotalCounters(PreparedStatement stmt, PreparedStatement errStmt, 
                                   ExtendedCountersSet set, long period, 
                                   ExportResults results) throws SQLException
    {
        setValues(stmt, period, null, set);
        stmt.executeUpdate(); results.total.records++;
        for (Iterator eit=set.getErrors().iterator(); eit.hasNext();) {
            ErrorCounterSet err = (ErrorCounterSet)eit.next();
            if (err == null) continue;
            setError(errStmt, period, null, err);
            errStmt.executeUpdate();
            results.total.errors++;
        }
    }
    private void dumpSmeCounters(PreparedStatement stmt, PreparedStatement errStmt, 
                                 HashMap map, long period,
                                 ExportResults results) throws SQLException
    {
        for (Iterator it=map.values().iterator(); it.hasNext();) {
            SmeIdCountersSet sme = (SmeIdCountersSet)it.next();
            if (sme == null) continue;
            setValues(stmt, period, sme.smeid, sme);
            stmt.executeUpdate();
            results.smes.records++;
            for (Iterator eit=sme.getErrors().iterator(); eit.hasNext();) {
                ErrorCounterSet err = (ErrorCounterSet)eit.next();
                if (err == null) continue;
                setError(errStmt, period, sme.smeid, err);
                errStmt.executeUpdate();
                results.smes.errors++;
            }
        }
    }
    private void dumpRouteCounters(PreparedStatement stmt, PreparedStatement errStmt,
                                   HashMap map, long period,
                                   ExportResults results) throws SQLException
    {
        for (Iterator it=map.values().iterator(); it.hasNext();) {
            RouteIdCountersSet route = (RouteIdCountersSet)it.next();
            if (route == null) continue;
            setValues(stmt, period, route.routeid, route);
            stmt.executeUpdate();
            results.routes.records++;
            for (Iterator eit=route.getErrors().iterator(); eit.hasNext();) {
                ErrorCounterSet err = (ErrorCounterSet)eit.next();
                if (err == null) continue;
                setError(errStmt, period, route.routeid, err);
                errStmt.executeUpdate();
                results.routes.errors++;
            }
        }
    }

    private void _exportStatistics(StatQuery query, ExportResults results,
                                   ExportSettings export) throws AdminException
    {
        final String tablesPrefix = export.getTablesPrefix();
        final String totalSmsTable = tablesPrefix + "_sms";
        final String totalErrTable = tablesPrefix + "_state";
        final String smeSmsTable   = tablesPrefix + "_sme";
        final String smeErrTable   = tablesPrefix + "_sme_state";
        final String routeSmsTable = tablesPrefix + "_route";
        final String routeErrTable = tablesPrefix + "_route_state";

        long fromPeriod = query.isFromDateEnabled() ? calculatePeriod(query.getFromDate()) : -1;
        long tillPeriod = query.isTillDateEnabled() ? calculatePeriod(query.getTillDate()) : -1;

        ConnectionPool pool = null; String errMessage = null;
        Connection connection = null; Statement stmt = null;
        PreparedStatement insertSms = null; PreparedStatement insertErr = null;
        PreparedStatement insertSmeSms = null; PreparedStatement insertSmeErr = null;
        PreparedStatement insertRouteSms = null; PreparedStatement insertRouteErr = null;
        try
        {   // create DS by export & obtain connection from it
            errMessage = "Failed to init & connect to DataSource";
            pool = createDataSource(export); connection = pool.getConnection();
            if (connection == null) throw new SQLException("Failed to obtain connection");

            // delete old statistics data
            errMessage = "Failed to drop old statistics data";
            stmt = connection.createStatement();
            final String wherePart = prepareWherePart(fromPeriod, tillPeriod);
            stmt.executeUpdate(DELETE_OP_SQL + totalSmsTable + wherePart);
            stmt.executeUpdate(DELETE_OP_SQL + totalErrTable + wherePart);
            stmt.executeUpdate(DELETE_OP_SQL + smeSmsTable + wherePart);
            stmt.executeUpdate(DELETE_OP_SQL + smeErrTable + wherePart);
            stmt.executeUpdate(DELETE_OP_SQL + routeSmsTable + wherePart);
            stmt.executeUpdate(DELETE_OP_SQL + routeErrTable + wherePart);
            connection.commit(); // commit old stat delete
            logger.debug("Old statistics data deleted");

            initQueryPeriod(query);
            TreeMap selectedFiles = getStatQueryDirs();
            if (selectedFiles == null || selectedFiles.size() <= 0) return;

            // perepare statements for Insert operations
            errMessage = "Failed to create insert statements";
            insertSms = connection.prepareStatement(INSERT_OP_SQL + totalSmsTable + VALUES_SMS_SQL);
            insertErr = connection.prepareStatement(INSERT_OP_SQL + totalErrTable + VALUES_SMS_ERR_SQL);
            insertSmeSms = connection.prepareStatement(INSERT_OP_SQL + smeSmsTable + VALUES_SME_SQL);
            insertSmeErr = connection.prepareStatement(INSERT_OP_SQL + smeErrTable + VALUES_SME_ERR_SQL);
            insertRouteSms = connection.prepareStatement(INSERT_OP_SQL + routeSmsTable + VALUES_ROUTE_SQL);
            insertRouteErr = connection.prepareStatement(INSERT_OP_SQL + routeErrTable + VALUES_ROUTE_ERR_SQL);
            errMessage = null;

            long tm = System.currentTimeMillis(); boolean finished = false;
            for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext() && !finished;)
            {
                Date fileDate = (Date) iterator.next(); // GMT
                String path = (String) selectedFiles.get(fileDate);
                logger.debug("Parsing file '"+path+"' ...");
                errMessage = "Failed to read statistics file '"+path+"'";
                InputStream input = null;
                try
                {   // reading stat file
                    input = new BufferedInputStream(new FileInputStream(path));
                    String fileStamp = readString(input, 9);
                    if (fileStamp == null || !fileStamp.equals("SMSC.STAT"))
                        throw new AdminException("unsupported header of file (support only SMSC.STAT file )");
                    readUInt16(input); // read version for support reasons

                    boolean haveValues = false;
                    ExtendedCountersSet totalCounters = new ExtendedCountersSet();
                    HashMap routeCounters = new HashMap();
                    HashMap smeCounters = new HashMap();
                    
                    byte buffer[] = new byte[512 * 1024];
                    Date curDate = null; Date lastDate = null; 
                    int recordNum = 0; int prevHour = -1;
                    
                    while (true) // iterate file records (by minutes)
                    {
                        try { // read record from file to buffer
                            recordNum++;
                            int rs1 = (int) readUInt32(input);
                            if (buffer.length < rs1) buffer = new byte[rs1];
                            Functions.readBuffer(input, buffer, rs1);
                            int rs2 = (int) readUInt32(input);
                            if (rs1 != rs2)
                                throw new IOException("Invalid file format "+path+" rs1=" + rs1 +
                                                      ", rs2=" + rs2+" at record="+recordNum);

                            ByteArrayInputStream is = new ByteArrayInputStream(buffer, 0, rs1);
                            try { // read record from buffer
                                int hour = readUInt8(is);
                                int min = readUInt8(is); min = 0; // skip minute
                                calendar.setTime(fileDate);
                                calendar.set(Calendar.HOUR, hour);
                                calendar.set(Calendar.MINUTE, min);
                                curDate = calendar.getTime();

                                if (fromQueryDate != null && curDate.getTime() < fromQueryDate.getTime()) {
                                    //logger.debug("Hour: "+hour+" skipped");
                                    continue;
                                }
                                
                                if (prevHour == -1) prevHour = hour;
                                if (lastDate == null) lastDate = curDate;

                                if (hour != prevHour && haveValues) { // switch to new hour
                                    logger.debug("New hour: " + hour + ", dump stat for: " + dateDayFormat.format(lastDate) + " GMT");
                                    errMessage = "Failed to dump statistics data";
                                    
                                    long period = calculatePeriod(lastDate); // dump counters to DB
                                    dumpTotalCounters(insertSms, insertErr, totalCounters, period, results);
                                    dumpSmeCounters(insertSmeSms, insertSmeErr, smeCounters, period, results);
                                    dumpRouteCounters(insertRouteSms, insertRouteErr, routeCounters, period, results);
                                    connection.commit(); // commit inserted data for period
                                    
                                    haveValues = false; lastDate = curDate; prevHour = hour;
                                    totalCounters = new ExtendedCountersSet();
                                    routeCounters = new HashMap();
                                    smeCounters = new HashMap();
                                    errMessage = null;
                                }
                                
                                if (tillQueryDate != null && curDate.getTime() >= tillQueryDate.getTime()) {
                                    finished = true;
                                    break; // finish work
                                }

                                haveValues = true; // read and increase counters
                                errMessage = "Failed to read statistics file '"+path+"'";
                                scanCounters(totalCounters, is);
                                scanErrors(totalCounters, is);
                                scanSmes(smeCounters, is);
                                scanRoutes(routeCounters, is);
                                errMessage = null;

                            } catch (EOFException exc) {
                                logger.warn("Incomplete record #"+recordNum+" in "+path+"");
                            }
                        } catch (EOFException exc) {
                            break; // current file ends
                        }
                    } // while has more records in file 
                    if (haveValues) { // dump the rest of data
                        if (logger.isDebugEnabled())
                            logger.debug("Last dump stat for: " + dateDayFormat.format(lastDate) + " GMT");
                        long period = calculatePeriod(lastDate); // dump counters to DB
                        dumpTotalCounters(insertSms, insertErr, totalCounters, period, results);
                        dumpSmeCounters(insertSmeSms, insertSmeErr, smeCounters, period, results);
                        dumpRouteCounters(insertRouteSms, insertRouteErr, routeCounters, period, results);
                        connection.commit(); // commit inserted data for period
                    }
                }
                catch (IOException e) { // stream fails or file has incorrect header
                    throw new AdminException(e.getMessage());
                } finally { // close stream
                    try { if (input != null) input.close(); } catch (Throwable th) {
                       logger.error("Failed to close input file stream", th);
                    }
                }
                logger.debug("File '"+path+"' parsed Ok");
            } // for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext();)
            logger.debug("End dumping statistics at: " + new Date() + " time spent: " +
                         (System.currentTimeMillis() - tm) / 1000);
        }
        catch (Exception exc) { // global cacth to close transaction
            rollbackConnection(connection);
            final String err = (errMessage != null) ? (errMessage+". Details: ") : "";
            throw new AdminException(err+exc.getMessage());
        }
        finally { // close connection & stmt(s)
            closeStatement(stmt);
            closeStatement(insertSms); closeStatement(insertErr);
            closeStatement(insertSmeSms); closeStatement(insertSmeErr);
            closeStatement(insertRouteSms); closeStatement(insertRouteErr);
            closeConnection(connection);
            shutdownPool(pool); pool = null;
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
}
