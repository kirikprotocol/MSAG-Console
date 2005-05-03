package ru.novosoft.smsc.admin.smsstat;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.smsview.DateConvertor;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.Functions;

import java.io.*;
import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 22.04.2005
 * Time: 15:32:13
 * To change this template use File | Settings | File Templates.
 */
public class SmsStat {
  org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(SmsStat.class);

  private final static String PARAM_NAME_STAT_DIR = "MessageStore.statisticsDir";

  private final static String DIR_SEPARATOR = "/";
  private final static String DATE_DIR_FORMAT = "yyyy-MM";
  private final static String DATE_DAY_FORMAT = "yyyy-MM-dd hh:mm";
  private final static String DATE_DIR_FILE_FORMAT = DATE_DIR_FORMAT + DIR_SEPARATOR + "dd";
  private final static String DATE_FILE_EXTENSION = ".rts";

  private String statstorePath;

  Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
  Calendar localCaledar = Calendar.getInstance(TimeZone.getDefault());

  //SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT);
  SimpleDateFormat dateDirFormat = new SimpleDateFormat(DATE_DIR_FORMAT);
  SimpleDateFormat dateDirFileFormat = new SimpleDateFormat(DATE_DIR_FILE_FORMAT);

  SimpleDateFormat dateDayFormat = new SimpleDateFormat(DATE_DAY_FORMAT);
  SimpleDateFormat dateDayLocalFormat = new SimpleDateFormat(DATE_DAY_FORMAT);

  Date fromQueryDate = null;
  Date tillQueryDate = null;

  private Statistics stat = null;
  private Smsc smsc = null;

  protected RouteSubjectManager routeSubjectManager = null;
  protected ProviderManager providerManager = null;
  protected CategoryManager categoryManager = null;

  public void init(SMSCAppContext appContext) throws AdminException {
    Smsc smsc = appContext.getSmsc();
    Config config = smsc.getSmscConfig();

    try {
      statstorePath = config.getString(PARAM_NAME_STAT_DIR);
    } catch (Exception e) {
      throw new AdminException("Failed to obtain statistics dir. Details: " + e.getMessage());
    }

    dateDirFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
    dateDirFileFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
    dateDayFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
    dateDayLocalFormat.setTimeZone(TimeZone.getDefault());
  }

  private TreeMap getStatQueryDirs() throws AdminException {
    File filePath = new File(statstorePath);
    String[] dirNames = filePath.list();
    if (dirNames == null || dirNames.length == 0)
      throw new AdminException("No statistics directories");

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

      String currentDir = statstorePath + DIR_SEPARATOR + dirName;
      File[] dirFiles = (new File(currentDir)).listFiles();
      if (dirFiles == null || dirFiles.length == 0) continue;

      for (int j = 0; j < dirFiles.length; j++) {
        String fileName = dirFiles[j].getName();
        if (fileName == null || fileName.length() <= 0 ||
                !fileName.toLowerCase().endsWith(DATE_FILE_EXTENSION))
          continue;

        String dirFileName = dirName + DIR_SEPARATOR + fileName;
        Date fileDate;
        try {
          fileDate = dateDirFileFormat.parse(dirFileName);
        } catch (ParseException exc) {
          continue;
        }
        if (fromQueryFileTime != null && fileDate.getTime() < fromQueryFileTime.getTime()) continue;
        if (tillQueryFileTime != null && fileDate.getTime() > tillQueryFileTime.getTime()) continue;

        selected.put(fileDate, statstorePath + DIR_SEPARATOR + dirFileName);
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

  public Statistics getStatistics(StatQuery query) throws AdminException {
    stat = new Statistics();

    initQueryPeriod(query);
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

    TreeMap selectedFiles = getStatQueryDirs();
    if (selectedFiles == null || selectedFiles.size() <= 0) return stat;

    TreeMap statByHours = new TreeMap(); // to add lastHourCounter to it
    HashMap countersForSme = new HashMap(); // contains SmeIdCountersSet
    HashMap countersForRoute = new HashMap(); // contains RouteIdCountersSet
    // System.out.println("Start scanning statistics at: " + new Date());
    long tm = System.currentTimeMillis();

    boolean finished = false;
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
    logger.debug("End scanning statistics at: " + new Date() + " time spent: " + (System.currentTimeMillis() - tm) / 1000);


    DateCountersSet dateCounters = null;
    Date lastDate = null;
    for (Iterator it = statByHours.keySet().iterator(); it.hasNext();) {
      Date hourDate = (Date) it.next();
      CountersSet hourCounter = (CountersSet) statByHours.get(hourDate);
      localCaledar.setTime(hourDate);
      int hour = localCaledar.get(Calendar.HOUR_OF_DAY);

      if (lastDate == null || hourDate.getTime() - lastDate.getTime() >= Constants.Day) {
        localCaledar.set(Calendar.HOUR_OF_DAY, 0);
        localCaledar.set(Calendar.MINUTE, 0);
        localCaledar.set(Calendar.SECOND, 0);
        localCaledar.set(Calendar.MILLISECOND, 0);
        lastDate = localCaledar.getTime();
        if (dateCounters != null) stat.addDateStat(dateCounters);
        dateCounters = new DateCountersSet(lastDate);
        String lastDateLocalStr = dateDayLocalFormat.format(lastDate);
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

  public static int readUInt8(InputStream is) throws IOException {
    int b = is.read();
    if (b == -1) throw new EOFException();
    return b;
  }

  public static int readUInt16(InputStream is) throws IOException {
    return (readUInt8(is) << 8 | readUInt8(is));
  }

  public static long readUInt32(InputStream is) throws IOException {
    return ((long) readUInt8(is) << 24) | ((long) readUInt8(is) << 16) |
            ((long) readUInt8(is) << 8) | ((long) readUInt8(is));
  }

  public static long readInt64(InputStream is) throws IOException {
    return (readUInt32(is) << 32) | readUInt32(is);
  }

  public static String readString(InputStream is, int size) throws IOException {
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

  public static void skipBytes(InputStream is, int size) throws IOException {
    int pos = 0;
    int cnt = 0;
    while (pos < size) {
      cnt = is.read();
      if (cnt == -1) throw new EOFException();
      pos += 1;
    }
  }

  public void setCategoryManager(CategoryManager categoryManager) {
    this.categoryManager = categoryManager;
  }

  public void setProviderManager(ProviderManager providerManager) {
    this.providerManager = providerManager;
  }

  public void setRouteSubjectManager(RouteSubjectManager routeSubjectManager) {
    this.routeSubjectManager = routeSubjectManager;
  }

  public void setSmsc(Smsc smsc) {
    this.smsc = smsc;
  }

  public Smsc getSmsc() {
    return smsc;
  }

}
