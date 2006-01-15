package ru.sibinco.scag.backend.stat.stat;

import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.scag.beans.MessageException;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.Constants;
import ru.sibinco.scag.backend.stat.stat.CountersSet;
import ru.sibinco.scag.backend.stat.stat.DateCountersSet;
import ru.sibinco.scag.backend.stat.stat.HourCountersSet;
import ru.sibinco.scag.backend.stat.stat.ExtendedCountersSet;
import ru.sibinco.scag.backend.stat.stat.SmeIdCountersSet;
import ru.sibinco.scag.backend.stat.stat.SmscIdCountersSet;
import ru.sibinco.scag.backend.stat.stat.RouteIdCountersSet;
import ru.sibinco.scag.backend.transport.Transport;


import java.util.Date;
import java.util.HashMap;
import java.util.Collection;
import java.util.TimeZone;
import java.util.Calendar;
import java.util.TreeMap;
import java.util.Iterator;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.io.File;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.ByteArrayInputStream;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 13.08.2004
 * Time: 16:00:56
 * To change this template use File | Settings | File Templates.
 */
public class Stat {

    org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(Stat.class);

    private final static String DATE_DIR_FORMAT = "yyyy-MM";
    private final static String DATE_DAY_FORMAT = "yyyy-MM-dd hh:mm";
    private final static String DATE_DIR_FILE_FORMAT = DATE_DIR_FORMAT + File.separatorChar + "dd";
    private final static String DATE_FILE_EXTENSION = ".rts";

    private Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
    private Calendar localCaledar = Calendar.getInstance(TimeZone.getDefault());
    private SimpleDateFormat dateDirFormat = new SimpleDateFormat(DATE_DIR_FORMAT);
    private SimpleDateFormat dateDirFileFormat = new SimpleDateFormat(DATE_DIR_FILE_FORMAT);
    private SimpleDateFormat dateDayFormat = new SimpleDateFormat(DATE_DAY_FORMAT);
    private SimpleDateFormat dateDayLocalFormat = new SimpleDateFormat(DATE_DAY_FORMAT);

    private String statstorePath;
    private final static String PARAM_NAME_STAT_DIR = "StatisticsManager.statisticsDir";

    private Date fromQueryDate = null;
    private Date tillQueryDate = null;

    private static Object instanceLock = new Object();
    private static Stat instance = null;

    public static Stat getInstance(Config gwConfig) throws Exception {
        synchronized (instanceLock) {
            if (instance == null) instance = new Stat(gwConfig);
            return instance;
        }
    }

    protected Stat(Config gwConfig) throws MessageException {
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
    }

    private TreeMap getStatQueryDirs(long transport) throws MessageException {

        File statPath = new File(statstorePath, Transport.getTransportName((int) transport));

        if (statPath == null || !statPath.isAbsolute()) {
            File gwConfFile = SCAGAppContext.getScagConfFolder();
            String gwDirFile = gwConfFile.getParent();
            statPath = new File(gwDirFile, statstorePath + File.separatorChar + Transport.getTransportName((int) transport));
            logger.debug("Stat path: by gateway conf '" + statPath.getAbsolutePath() + "'");
        } else {
            logger.debug("Stat path: is absolute '" + statPath.getAbsolutePath() + "'");
        }

        String[] dirNames = statPath.list();
        if (dirNames == null || dirNames.length == 0)
            throw new MessageException("No stat directories at path '" + statPath.getAbsolutePath() + "'");

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
            try {
                dirDate = dateDirFormat.parse(dirName);
            } catch (ParseException exc) {
                continue;
            }

            if (fromQueryDirTime != null && dirDate.getTime() < fromQueryDirTime.getTime()) continue;
            if (tillQueryDirTime != null && dirDate.getTime() > tillQueryDirTime.getTime()) continue;

            File dirNameFile = new File(statPath, dirName);
            File[] dirFiles = dirNameFile.listFiles();
            if (dirFiles == null || dirFiles.length == 0) continue;

            for (int j = 0; j < dirFiles.length; j++) {
                String fileName = dirFiles[j].getName();
                if (fileName == null || fileName.length() <= 0 ||
                        !fileName.toLowerCase().endsWith(DATE_FILE_EXTENSION))
                    continue;

                Date fileDate;
                try {
                    fileDate = dateDirFileFormat.parse(dirName + File.separatorChar + fileName);
                } catch (ParseException exc) {
                    continue;
                }

                if (fromQueryFileTime != null && fileDate.getTime() < fromQueryFileTime.getTime()) continue;
                if (tillQueryFileTime != null && fileDate.getTime() > tillQueryFileTime.getTime()) continue;

                final String fullFilePath = (new File(dirNameFile, fileName)).getAbsolutePath();
                selected.put(fileDate, fullFilePath);
            }
        }
        return selected;
    }

    private Object generalLock = new Object();

    public Statistics getStatistics(StatQuery query) throws Exception {
        synchronized (generalLock) {
            return _getStatistics(query);
        }
    }

    private void initQueryPeriod(StatQuery query) {
        fromQueryDate = query.isFromDateEnabled() ? query.getFromDate() : null;
        tillQueryDate = query.isTillDateEnabled() ? query.getTillDate() : null;
    }

    private Statistics _getStatistics(StatQuery query) throws Exception {

        initQueryPeriod(query);
        if (logger.isInfoEnabled()) {
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
        TreeMap selectedFiles = getStatQueryDirs(query.getTransport());
        if (selectedFiles == null || selectedFiles.size() <= 0) return stat;

        HashMap countersForSme = new HashMap(); // contains SmeIdCountersSet
        HashMap countersForRoute = new HashMap(); // contains RouteIdCountersSet
        HashMap countersForSmsc = new HashMap(); // contains SmscIdCountersSet
        TreeMap statByHours = new TreeMap(); // to add lastHourCounter to it


        long tm = System.currentTimeMillis();
        boolean finished = false;
        for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext() && !finished;) {
            Date fileDate = (Date) iterator.next(); // GMT
            String path = (String) selectedFiles.get(fileDate);
            {
                logger.debug("Parsing file: " + dateDayFormat.format(fileDate) + " GMT (" + dateDayLocalFormat.format(fileDate) + " local)");
            }
            InputStream input = null;
            try {
                input = new BufferedInputStream(new FileInputStream(path));
                String fileStamp = readString(input, 9); // read head: 9 bytes
                if (fileStamp == null || !fileStamp.equals("SCAG.STAT"))
                    throw new MessageException("unsupported header of file (support only SCAG.STAT file )");
                readUInt16(input); // read version for support reasons uint16

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
                        int rs1 = (int) readUInt32(input); // reading length uint32

                        if (buffer.length < rs1) buffer = new byte[rs1];
                        Functions.readBuffer(input, buffer, rs1);
                        int rs2 = (int) readUInt32(input);
                        if (rs1 != rs2)
                            throw new IOException("Invalid file format " + path + " rs1=" + rs1 + ", rs2=" + rs2 + " at record=" + recordNum);

                        ByteArrayInputStream is = new ByteArrayInputStream(buffer, 0, rs1);
                        try {
                            int hour = readUInt8(is);
                            int min = readUInt8(is);
                            calendar.setTime(fileDate);
                            calendar.set(Calendar.HOUR, hour);
                            calendar.set(Calendar.MINUTE, min);
                            curDate = calendar.getTime();

                            if (fromQueryDate != null && curDate.getTime() < fromQueryDate.getTime()) {
                                logger.debug("Hour: " + hour + " skipped");
                                continue;
                            }

                            if (prevHour == -1)
                                prevHour = hour;
                            if (lastDate == null)
                                lastDate = curDate;

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

                            scanRoutesAndGeneralStat(countersForRoute, lastHourCounter, is);
                            scanSmes(countersForSme, is);
                            scanSmsc(countersForSmsc, is);
                        } catch (EOFException exc) {
                            logger.warn("Incomplete record #" + recordNum + " in " + path + "");
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
                throw new MessageException(e.getMessage());
            } finally {
                try {
                    if (input != null) input.close();
                } catch (Throwable th) {
                    th.printStackTrace();
                }
            }
        } // end "for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext();)"

        logger.debug("End scanning statistics at: " + new Date() + " time spent: " +
                (System.currentTimeMillis() - tm) / 1000);

        DateCountersSet dateCounters = null;
        Date lastDate = null;
        for (Iterator it = statByHours.keySet().iterator(); it.hasNext();) {
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
            set.incrementFull(hourCounter);
            dateCounters.addHourStat(set);
        }
        if (dateCounters != null)
            stat.addDateStat(dateCounters);

        Collection countersSme = countersForSme.values();
        if (countersSme != null)
            stat.addSmeIdCollection(countersSme);
        Collection countersRoute = countersForRoute.values();
        if (countersRoute != null)
            stat.addRouteIdCollection(countersRoute);
        Collection countersSmsc = countersForSmsc.values();
        if (countersSmsc != null)
            stat.addSmscIdCollection(countersSmsc);
        return stat;
    }

    private void scanRouteAndGeneralStatCounters(CountersSet set, CountersSet countersSet, InputStream is) throws IOException {
        int accepted = (int) readUInt32(is);
        int rejected = (int) readUInt32(is);
        int delivered = (int) readUInt32(is);
        int gw_rejected = (int) readUInt32(is);
        int failed = (int) readUInt32(is);
        int billingOk = (int) readUInt32(is);
        int billingFailed = (int) readUInt32(is);
        int recieptOk = (int) readUInt32(is);
        int recieptFailed = (int) readUInt32(is);
        set.incrementFull(accepted, rejected, delivered, gw_rejected, failed, billingOk, billingFailed, recieptOk, recieptFailed);
        countersSet.incrementFull(accepted, rejected, delivered, gw_rejected, failed, billingOk, billingFailed, recieptOk, recieptFailed);
    }

    private void scanSmeCounters(CountersSet set, InputStream is) throws IOException {
        int accepted = (int) readUInt32(is);
        int rejected = (int) readUInt32(is);
        int delivered = (int) readUInt32(is);
        int gw_rejected = (int) readUInt32(is);
        int failed = (int) readUInt32(is);
        set.increment(accepted, rejected, delivered, gw_rejected, failed);
    }

    private void scanSmscCounters(CountersSet set, InputStream is) throws IOException {
        int accepted = (int) readUInt32(is);
        int rejected = (int) readUInt32(is);
        int delivered = (int) readUInt32(is);
        int gw_rejected = (int) readUInt32(is);
        int failed = (int) readUInt32(is);
        set.increment(accepted, rejected, delivered, gw_rejected, failed);
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
            int providerId = (int) readUInt32(is);
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

    private void scanSmsc(HashMap map, InputStream is) throws IOException {
        int counter = (int) readUInt32(is);
        while (counter-- > 0) {
            int smsc_id_len = readUInt8(is);
            String smscId = readString(is, smsc_id_len);

            int providerId = (int) readUInt32(is);

            SmscIdCountersSet set = (SmscIdCountersSet) map.get(smscId);
            if (set == null) {
                set = new SmscIdCountersSet(smscId);
                map.put(smscId, set);
            }
            set.setProviderId(providerId);
            scanSmscCounters(set, is);
            scanErrors(set, is);
        }
    }

    private void scanRoutesAndGeneralStat(HashMap map, CountersSet lastHourCounter, InputStream is) throws IOException {
        int counter = (int) readUInt32(is);
        while (counter-- > 0) {
            int route_id_len = readUInt8(is);
            String routeId = readString(is, route_id_len);

            int providerId = (int) readUInt32(is);

            RouteIdCountersSet set = (RouteIdCountersSet) map.get(routeId);
            if (set == null) {
                set = new RouteIdCountersSet(routeId);
                map.put(routeId, set);
            }
            set.setProviderId(providerId);
            scanRouteAndGeneralStatCounters(set, lastHourCounter, is);
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
}

