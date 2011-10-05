package ru.novosoft.smsc.admin.stat;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.IOUtils;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmscStatProvider {

  private static final Logger logger = Logger.getLogger(SmscStatProvider.class);

  private final static String DATE_DIR_FORMAT = "yyyy-MM";
  private final static String DATE_DAY_FORMAT = "yyyy-MM-dd HH:mm";
  private final static String DATE_DIR_FILE_FORMAT = DATE_DIR_FORMAT + File.separatorChar + "dd";
  private final static String DATE_FILE_EXTENSION  = ".rts";

  private final SmscStatContext context;

  private final TimeZone defTimeZone = TimeZone.getDefault();
  private final TimeZone gmtTimeZone = TimeZone.getTimeZone("GMT");

  public SmscStatProvider(SmscStatContext context) {
    this.context = context;
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
    if(date == null) {
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
    if(date == null) {
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



  private void getStatFiles(File statPath, Collection<FileWithDate> result, Date fromDate, Date tillDate) throws AdminException {

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

      if(dirFromDate != null && dirDate.compareTo(dirFromDate) < 0) continue;
      if(dirTillDate != null && dirDate.compareTo(dirTillDate) > 0) continue;


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

        if(fileFromDate != null && fileDate.compareTo(fileFromDate) < 0) continue;
        if(fileTillDate != null && fileDate.compareTo(fileTillDate) > 0) continue;

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

    for(File statPath : context.getStatDirs()) {
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

  private boolean processFile(FileWithDate fwd, Date fromQueryDate, Date tillQueryDate, Statistics stat,
                              Map<Date, CountersSet> statByHours, Map<String, SmeIdCountersSet> countersForSme, Map<String,
      RouteIdCountersSet> countersForRoute) throws AdminException{

    SimpleDateFormat dateDayFormat = new SimpleDateFormat(DATE_DAY_FORMAT);

    boolean result = true;

    Date fileDate = fwd.date; // GMT
    File path = fwd.file;
    if(logger.isDebugEnabled()){
      logger.debug("Parsing file: "+dateDayFormat.format(fileDate)+" GMT");
    }
    InputStream input = null;
    try {
      input = new BufferedInputStream(context.getFileSystem().getInputStream(path));
      checkFileHeader(input, path.getAbsolutePath());
      CountersSet lastHourCounter = new CountersSet();
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
              if(logger.isDebugEnabled()) {
                logger.debug("New hour: " + hour + ", dump stat for: " + dateDayFormat.format(lastDate) + " GMT");
              }
              CountersSet hcs = statByHours.get(lastDate);
              if(hcs == null) {
                statByHours.put(lastDate, lastHourCounter);
              }else {
                hcs.increment(lastHourCounter);
              }
              haveValues = false;
              lastDate = curDate;
              prevHour = hour;
              lastHourCounter = new CountersSet();
            }

            if (tillQueryDate != null && curDate.getTime() >= tillQueryDate.getTime()) {
              result = false;
              break; // finish work
            }


            haveValues = true; // read and increase counters
            readCounters(lastHourCounter, is);
            readErrors(stat, is);
            readSmes(countersForSme, is);
            readRoutes(countersForRoute, is);
          } catch (EOFException exc) {
            logger.warn("Incomplete record #"+recordNum+" in '"+path.getAbsolutePath()+'\'');
          }
        } catch (EOFException exc) {
          break;
        }
      }
      if (haveValues) {
        if(logger.isDebugEnabled()) {
          logger.debug("Last dump stat for: " + dateDayFormat.format(lastDate) + " GMT");
        }
        CountersSet hcs = statByHours.get(lastDate);
        if(hcs == null) {
          statByHours.put(lastDate, lastHourCounter);
        }else {
          hcs.increment(lastHourCounter);
        }
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

    Statistics stat = new Statistics();
    Set<FileWithDate> selectedFiles = getStatFiles(fromQueryDate, tillQueryDate);
    if (selectedFiles.size() <= 0) return stat;

    Map<Date, CountersSet> statByHours = new TreeMap<Date, CountersSet>();
    Map<String, SmeIdCountersSet> countersForSme = new HashMap<String, SmeIdCountersSet>();
    Map<String, RouteIdCountersSet> countersForRoute = new HashMap<String, RouteIdCountersSet>();

    long tm = System.currentTimeMillis();  boolean finished = false;
    for (Iterator<FileWithDate> iterator = selectedFiles.iterator(); iterator.hasNext() && !finished;) {
      FileWithDate fwd = iterator.next();
      processFile(fwd, fromQueryDate, tillQueryDate, stat, statByHours, countersForSme, countersForRoute);
    }
    if(logger.isDebugEnabled()) {
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

    return stat;
  }


  private static void readBuffer(InputStream is, byte buffer[], int size) throws IOException {
    int read = 0;
    while (read < size) {
      int result = is.read(buffer, read, size - read);
      if (result < 0) throw new EOFException("Failed to read " + size + " bytes, read failed at " + read);
      read += result;
    }
  }

  private static class FileWithDate implements Comparable<FileWithDate>{
    private final File file;
    private final Date date;  //GMT
    private FileWithDate(File file, Date date) {
      this.file = file;
      this.date = date;
    }
    public int compareTo(FileWithDate o) {
      int res =  this.date.compareTo(o.date);
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



}
