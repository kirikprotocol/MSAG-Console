package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;

/**
 * author: Aleksandr Khalitov
 */
class MCISmeStatistics {

  private static final String SMSC_MCISME_STAT_HEADER_TEXT = "SMSC.MCISME.STAT";
  private static final int SMSC_MCISME_STAT_VERSION_INFO = 0x0001;

  private final File statPath;

  private final FileSystem fileSystem;

  MCISmeStatistics(File statPath, FileSystem fileSystem) {
    this.statPath = statPath;
    this.fileSystem = fileSystem;
  }

  private static Date resetByHour(Date date) {
    Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    return c.getTime();
  }

  private static Date getFromDate(StatFilter filter) {
    if(filter != null && filter.getFromDate() != null) {
      return resetByHour(filter.getFromDate());
    }
    return null;
  }


  private static Date getFromByDay(StatFilter filter) {
    if(filter == null || filter.getFromDate() == null) {
      return null;
    }
    final SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
    try{
      return format.parse(format.format(filter.getFromDate()));
    }catch (ParseException ignored) {
      return null;
    }

  }

  private static Date getFromByMonth(StatFilter filter) {
    if(filter == null || filter.getFromDate() == null) {
      return null;
    }
    final SimpleDateFormat format = new SimpleDateFormat("yyyy-MM");
    try{
      return format.parse(format.format(filter.getFromDate()));
    }catch (ParseException ignored) {
      return null;
    }
  }

  private static Date getTillByDay(StatFilter filter) {
    if(filter == null || filter.getTillDate() == null) {
      return null;
    }
    final SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
    try{
      return format.parse(format.format(filter.getTillDate()));
    }catch (ParseException ignored) {
      return null;
    }
  }

  private static Date getTillByMonth(StatFilter filter) {
    if(filter == null || filter.getTillDate() == null) {
      return null;
    }
    final SimpleDateFormat format = new SimpleDateFormat("yyyy-MM");
    try{
      return format.parse(format.format(filter.getTillDate()));
    }catch (ParseException ignored) {
      return null;
    }
  }

  private static Date getTillDate(StatFilter filter) {
    if(filter != null && filter.getTillDate() != null) {
      return resetByHour(filter.getTillDate());
    }
    return null;
  }

  public MessageStatistics getStatistics(StatFilter filter) throws AdminException {
    MessageStatistics statistics = new MessageStatistics();

    if (!fileSystem.exists(statPath)) {
      return statistics;
    }



    File[] dirs = fileSystem.listFiles(statPath, new FileFilter() {
      public boolean accept(File pathname) {
        return fileSystem.isDirectory(pathname);
      }
    });

    if(dirs == null || dirs.length == 0) {
      return null;
    }
    Arrays.sort(dirs);


    final SimpleDateFormat dirFormat = new SimpleDateFormat("yyyy-MM");

    Date fromDate = getFromDate(filter);
    Date tillDate = getTillDate(filter);
    Date fromByMonth = getFromByMonth(filter);
    Date tillByMonth = getTillByMonth(filter);
    Date fromByDay = getFromByDay(filter);
    Date tillByDay = getTillByDay(filter);


    for(File dir : dirs) {
      Date dirDate;
      try {
        dirDate = dirFormat.parse(dir.getName());
      } catch (ParseException e) {
        continue;
      }
      if(fromByMonth != null && dirDate.compareTo(fromByMonth)<0) {
        continue;
      }
      if(tillByMonth != null && dirDate.compareTo(tillByMonth)>0) {
        break;
      }
      File[] days = fileSystem.listFiles(dir, new FileFilter() {
        public boolean accept(File pathname) {
          return !fileSystem.isDirectory(pathname);
        }
      });
      if(days == null || days.length == 0) {
        continue;
      }
      Arrays.sort(days);
      for(File day1 : days) {
        int day = Integer.parseInt(day1.getName().substring(0, 2));
        Calendar fileCal = Calendar.getInstance();
        fileCal.setTime(dirDate);
        fileCal.set(Calendar.DAY_OF_MONTH, day);
        Date fileDate = fileCal.getTime();
        if(fromByDay != null && fileDate.compareTo(fromByDay)<0) {
          continue;
        }
        if(tillByDay != null && fileDate.compareTo(tillByDay)>0) {
          break;
        }
        readFile(statistics, day1, fileDate, fromDate, tillDate);
      }
    }
    return statistics;
  }

  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  private static boolean checkVersion(InputStream statFile) throws IOException {
    byte[] buff = new byte[SMSC_MCISME_STAT_HEADER_TEXT.length()];
    statFile.read(buff, 0, SMSC_MCISME_STAT_HEADER_TEXT.length());
    String header = new String(buff);
    if (SMSC_MCISME_STAT_HEADER_TEXT.compareTo(header) != 0) return false;
    statFile.read(buff, 0, 2);
    int version = buff[0];
    version = (version << 8) | buff[1];
    if (version != SMSC_MCISME_STAT_VERSION_INFO) return false;
    return true;
  }

  private static Date setHour(Date date, int hour) {
    Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.HOUR_OF_DAY, hour);
    return c.getTime();
  }

  private  void readFile(MessageStatistics statistics, File day1, Date fileDate, Date fromDate, Date tillDate) throws AdminException {
    DateCountersSet dayStat = new DateCountersSet(fileDate);
    InputStream statFile = null;
    try{
      statFile = fileSystem.getInputStream(day1);
      if(!checkVersion(statFile)) {
        return;
      }
      CountersSet counters = new CountersSet();
      CountersSet curCounters = new CountersSet();
      int hour = getNextHourStat(statFile, counters);
      int curHour = hour;

      while (curHour != -1) {
        if (hour != curHour) {
          Date curDate = setHour(fileDate, curHour);
          if (((fromDate == null || curDate.compareTo(fromDate) >= 0) && (tillDate == null || curDate.compareTo(tillDate) <= 0))) {
            HourCountersSet hourStat = new HourCountersSet(curCounters, curHour);
            dayStat.addHourStat(hourStat);
          }
          curHour = hour;
          curCounters.reset();
        } else {
          curCounters.increment(counters);
        }
        if (hour != -1) {
          hour = getNextHourStat(statFile, counters);
        }
      }
      statistics.addDateStat(dayStat);
    } catch (IOException ignored) {
    } finally {
      if(statFile != null) {
        try{
          statFile.close();
        }catch (IOException ignored){}
      }
    }
  }

  private static int getNextHourStat(InputStream statFile, CountersSet counters) {
    int hour;
    try {
      byte[] buff = new byte[17];//[25];
      if (statFile.read(buff, 0, 17) == -1) return -1;
      hour = buff[0];//4];
      counters.setMissed(byte4_to_int(buff, 1));//5);
      counters.setDelivered(byte4_to_int(buff, 5));//);
      counters.setFailed(byte4_to_int(buff, 9));//13);
      counters.setNotified(byte4_to_int(buff, 13));//17);
    } catch (Exception e) {
      return -1;
    }
    return hour;
  }

  private static long byte4_to_int(byte[] arr, int from) {
    long res = 0;
    for (int i = 0; i < 4; i++) {
      res = add_byte(res, arr[from + i]);
    }
    return res;
  }

  private static long add_byte(long value, byte num) {
    long res;
    if (num < 0) {
      res = (value << 8) | (num & 0x7F);
      res = res | 0x80;
    } else {
      res = (value << 8) | num;
    }

    return res;
  }


}
