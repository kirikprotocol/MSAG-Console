import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

public class ExportStat {
  public class CountersSet {
    public long accepted = 0;
    public long rejected = 0;
    public long delivered = 0;
    public long failed = 0;
    public long rescheduled = 0;
    public long temporal = 0;
    public long peak_i = 0;
    public long peak_o = 0;

    public CountersSet() {
    }

    public CountersSet(long accepted, long rejected, long delivered,
                       long failed, long rescheduled, long temporal,
                       long i, long o) {
      this.increment(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    }

    protected void increment(CountersSet set) {
      this.accepted += set.accepted;
      this.rejected += set.rejected;
      this.delivered += set.delivered;
      this.failed += set.failed;
      this.rescheduled += set.rescheduled;
      this.temporal += set.temporal;
      if (set.peak_i > this.peak_i) this.peak_i = set.peak_i;
      if (set.peak_o > this.peak_o) this.peak_o = set.peak_o;
    }

    protected void increment(long accepted, long rejected, long delivered,
                             long failed, long rescheduled, long temporal,
                             long i, long o) {
      this.accepted += accepted;
      this.rejected += rejected;
      this.delivered += delivered;
      this.failed += failed;
      this.rescheduled += rescheduled;
      this.temporal += temporal;
      if (i > this.peak_i) this.peak_i = i;
      if (o > this.peak_o) this.peak_o = o;
    }

    public void reset() {
      accepted = 0;
      rejected = 0;
      delivered = 0;
      failed = 0;
      rescheduled = 0;
      temporal = 0;
      peak_i = 0;
      peak_o = 0;
    }
  }

  public class ErrorCounterSet {
    public int errcode;
    public long counter;

    ErrorCounterSet(int err, long count) {
      errcode = err;
      counter = count;
    }

    protected void increment(long count) {
      this.counter += count;
    }
  }

  public class ExtendedCountersSet extends CountersSet {
    private TreeMap errors = new TreeMap();

    public ExtendedCountersSet() {
    }

    public ExtendedCountersSet(long accepted, long rejected, long delivered,
                               long failed, long rescheduled, long temporal, long i, long o) {
      super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    }

    public void incError(int errcode, long count) {
      Integer key = new Integer(errcode);
      ErrorCounterSet set = (ErrorCounterSet) errors.get(key);
      if (set == null)
        errors.put(key, new ErrorCounterSet(errcode, count));
      else
        set.increment(count);
    }

    public void addAllErr(Collection err) {
      for (Iterator i = err.iterator(); i.hasNext();) {
        ErrorCounterSet set = (ErrorCounterSet) i.next();
        if (set != null) errors.put(new Integer(set.errcode), set);
      }
    }

    public Collection getErrors() {
      return errors.values();
    }
  }

  public class SmeIdCountersSet extends ExtendedCountersSet implements Comparable {
    public String smeid;

    public SmeIdCountersSet(String smeid) {
      this.smeid = smeid;
    }

    public SmeIdCountersSet(long accepted, long rejected, long delivered,
                            long failed, long rescheduled, long temporal, long i, long o, String smeid) {
      super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
      this.smeid = smeid;
    }

    public int compareTo(Object o) {
      if (o == null || smeid == null || !(o instanceof SmeIdCountersSet)) return -1;
      return smeid.compareTo(((SmeIdCountersSet) o).smeid);
    }
  }

  public class RouteIdCountersSet extends ExtendedCountersSet implements Comparable {
    public String routeid;
    public long providerId = -1;
    public long categoryId = -1;
    private Object provider = null;
    private Object category = null;

    public RouteIdCountersSet(String routeid) {
      this.routeid = routeid;
    }

    public RouteIdCountersSet(long accepted, long rejected, long delivered,
                              long failed, long rescheduled, long temporal, long i, long o, String routeid) {
      super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
      this.routeid = routeid;
    }

    public RouteIdCountersSet(long accepted, long rejected, long delivered,
                              long failed, long rescheduled, long temporal, long i, long o, String routeid,
                              long providerId, long categoryId) {
      super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
      this.routeid = routeid;
      this.providerId = providerId;
      this.categoryId = categoryId;
    }

    public void setCategoryId(long categoryId) {
      this.categoryId = categoryId;
    }

    public void setProviderId(long providerId) {
      this.providerId = providerId;
    }

    public int compareTo(Object o) {
      if (o == null || routeid == null || !(o instanceof RouteIdCountersSet)) return -1;
      return routeid.compareTo(((RouteIdCountersSet) o).routeid);
    }

    public Object getCategory() {
      return category;
    }

    public void setCategory(Object category) {
      this.category = category;
    }

    public Object getProvider() {
      return provider;
    }

    public void setProvider(Object provider) {
      this.provider = provider;
    }
  }

  private final static String DATE_DIR_FORMAT = "yyyy-MM";
  private final static String DATE_DAY_FORMAT = "yyyy-MM-dd HH:mm";
  private final static String DATE_DIR_FILE_FORMAT = DATE_DIR_FORMAT + File.separatorChar + "dd";
  private final static String DATE_FILE_EXTENSION = ".rts";

  private Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
  private Calendar localCalendar = Calendar.getInstance(TimeZone.getDefault());
  private SimpleDateFormat dateDirFormat = new SimpleDateFormat(DATE_DIR_FORMAT);
  private SimpleDateFormat dateDirFileFormat = new SimpleDateFormat(DATE_DIR_FILE_FORMAT);
  private SimpleDateFormat dateDayFormat = new SimpleDateFormat(DATE_DAY_FORMAT);
  private SimpleDateFormat gmtDateDayFormat = new SimpleDateFormat(DATE_DAY_FORMAT);

  private final static String DATE_FORMAT = "yyyy-MM-dd-HH-mm";
  private SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT);
  private SimpleDateFormat gmtDateFormat = new SimpleDateFormat(DATE_FORMAT);
  private final static String DEST_FILE_PREFIX_FORMAT = "yyyyMMdd_HH";
  private SimpleDateFormat destFilePrefixFormat = new SimpleDateFormat(DEST_FILE_PREFIX_FORMAT);
  private final static String DEST_DATE_FORMAT = "yyyyMMdd_HHmm";
  private SimpleDateFormat destDateFormat = new SimpleDateFormat(DEST_DATE_FORMAT);

  private File srcDir = null;
  private File destDir = null;
  private Date fromDate = null;
  private Date tillDate = null;
  private int  minuteInterval = 5;
  private boolean useFromDateInHeaders;
  private boolean writeTotals;

  public ExportStat(String src, String dest, String minuteInterval, String dateTime, String useFromDateInHeaders, String writeTotals) {
    srcDir = new File(src);
    destDir = new File(dest);
    try {
      if (useFromDateInHeaders != null) {
        this.useFromDateInHeaders = useFromDateInHeaders != null && useFromDateInHeaders.equals("true");
      } else if (dateTime != null) {
        if (dateTime.equals("false")) {
          this.useFromDateInHeaders = false;
          dateTime = null;
        } else if (dateTime.equals("true")) {
          this.useFromDateInHeaders = true;
          dateTime = null;
        }
      }

      if (writeTotals != null)
        this.writeTotals = writeTotals.equals("true");

      calendar.set(Calendar.DATE, 18);
      gmtDateFormat.setCalendar(calendar);
      gmtDateDayFormat.setCalendar(calendar);
      dateFormat.setCalendar(localCalendar);
      this.minuteInterval = Integer.valueOf(minuteInterval).intValue();
      if( dateTime != null ) {
        localCalendar.setTime(dateFormat.parse(dateTime));
        tillDate = localCalendar.getTime();
        localCalendar.add(Calendar.MINUTE, -this.minuteInterval);
        fromDate = localCalendar.getTime();
      } else {
        calendar.set(Calendar.SECOND, 0);
        calendar.set(Calendar.MILLISECOND,0);
        // round to interval
        calendar.set(Calendar.MINUTE, (calendar.get(Calendar.MINUTE)/this.minuteInterval)*this.minuteInterval);
        tillDate = calendar.getTime();
        calendar.add(Calendar.MINUTE, -this.minuteInterval);
        fromDate = calendar.getTime();
      }
      System.out.println("GMT from: " + gmtDateDayFormat.format(fromDate) + " till: " + gmtDateDayFormat.format(tillDate));
    }
    catch (java.text.ParseException e) {
      e.printStackTrace();
    }
  }

  public void process() {
    try {
      String errMessage = null;
      TreeMap selectedFiles = getStatQueryDirs();
      if (selectedFiles == null || selectedFiles.size() <= 0) return;

      long tm = System.currentTimeMillis();
      boolean finished = false;
      for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext() && !finished;) {
        Date fileDate = (Date) iterator.next(); // GMT
        String path = (String) selectedFiles.get(fileDate);
        System.out.println("Parsing file '" + path + "' ...");
        errMessage = "Failed to read statistics file '" + path + "'";
        InputStream input = null;
        try {   // reading stat file
          input = new BufferedInputStream(new FileInputStream(path));
          String fileStamp = readString(input, 9);
          if (fileStamp == null || !fileStamp.equals("SMSC.STAT"))
            throw new Exception("unsupported header of file (support only SMSC.STAT file )");
          readUInt16(input); // read version for support reasons

          boolean haveValues = false;
          ExtendedCountersSet totalCounters = new ExtendedCountersSet();
          HashMap routeCounters = new HashMap();
          HashMap smeCounters = new HashMap();

          byte buffer[] = new byte[512 * 1024];
          Date curDate = null;
          Date lastDate = null;
          int recordNum = 0;

          while (true) // iterate file records (by minutes)
          {
            try { // read record from file to buffer
              recordNum++;
              int rs1 = (int) readUInt32(input);
              System.out.println(rs1);
              if (buffer.length < rs1) buffer = new byte[rs1];
              readBuffer(input, buffer, rs1);
              int rs2 = (int) readUInt32(input);
              if (rs1 != rs2)
                throw new IOException("Invalid file format " + path + " rs1=" + rs1 +
                        ", rs2=" + rs2 + " at record=" + recordNum);

              ByteArrayInputStream is = new ByteArrayInputStream(buffer, 0, rs1);
              try { // read record from buffer
                int hour = readUInt8(is);
                int min = readUInt8(is);
                //min = 0; // skip minute
                calendar.setTime(fileDate);
                calendar.set(Calendar.HOUR, hour);
                calendar.set(Calendar.MINUTE, min);
                curDate = calendar.getTime();
//                System.out.println("Checking period "+gmtDateDayFormat.format(curDate));
                if (fromDate != null && curDate.getTime() < fromDate.getTime()) {
                  continue;
                }

                if (tillDate != null && curDate.getTime() >= tillDate.getTime()) {
                  finished = true;
                  break;
                }
//                System.out.println("Filling counters");
                haveValues = true; // read and increase counters
                errMessage = "Failed to read statistics file '" + path + "'";
                scanCounters(totalCounters, is);
                scanErrors(totalCounters, is);
                scanSmes(smeCounters, is);
                scanRoutes(routeCounters, is);
                errMessage = null;

              } catch (EOFException exc) {
                System.out.println("Incomplete record #" + recordNum + " in " + path + "");
              }
            } catch (EOFException exc) {
              break; // current file ends
            }
          } // while has more records in file
          if (haveValues) { // dump the rest of data
            dump(smeCounters, routeCounters, fromDate, tillDate);
            if (lastDate != null)
              System.out.println("Last dump stat for: " + dateDayFormat.format(lastDate) + " GMT");
          }
        }
        catch (IOException e) { // stream fails or file has incorrect header
          throw new Exception(e.getMessage());
        } finally { // close stream
          try {
            if (input != null) input.close();
          } catch (Throwable th) {
            System.out.println("Failed to close input file stream");
          }
        }
        System.out.println("File '" + path + "' parsed Ok");
      } // for (Iterator iterator = selectedFiles.keySet().iterator(); iterator.hasNext();)
      System.out.println("End dumping statistics at: " + new Date() + " time spent: " +
              (System.currentTimeMillis() - tm) / 1000);
    }
    catch (Exception exc) {
      exc.printStackTrace();
    }
  }

  private void dump(HashMap smeCounters, HashMap routeCounters, Date fromDate, Date tillDate) throws Exception {
    String datePrefix = destFilePrefixFormat.format(fromDate);
    File dest = new File(destDir, "sme_stats_" + datePrefix + ".csv");
    PrintWriter r = new PrintWriter(new FileWriter(dest, true));
    if (useFromDateInHeaders)
      r.println("#"+destDateFormat.format(fromDate));
    else
      r.println("#"+destDateFormat.format(tillDate));

    SmeIdCountersSet totalCounters = new SmeIdCountersSet("Total");
    for (Iterator i = smeCounters.keySet().iterator(); i.hasNext();) {
      String smeId = (String) i.next();
      SmeIdCountersSet set = (SmeIdCountersSet) smeCounters.get(smeId);
      if (set == null) continue;
      r.println(smeId + ",accepted," + set.accepted);
      totalCounters.accepted+=set.accepted;
      r.println(smeId + ",rejected," + set.rejected);
      totalCounters.rejected+=set.rejected;
      r.println(smeId + ",delivered," + set.delivered);
      totalCounters.delivered+=set.delivered;
      r.println(smeId + ",failed," + set.failed);
      totalCounters.failed+=set.failed;
      r.println(smeId + ",rescheduled," + set.rescheduled);
      totalCounters.rescheduled+=set.rescheduled;
      r.println(smeId + ",temporal," + set.temporal);
      totalCounters.temporal +=set.temporal;
      r.println(smeId + ",peak_i," + set.peak_i);
      totalCounters.peak_i += set.peak_i;
      r.println(smeId + ",peak_o," + set.peak_o);
      totalCounters.peak_o += set.peak_o;
      for (Iterator j = set.getErrors().iterator(); j.hasNext();) {
        ErrorCounterSet err = (ErrorCounterSet) j.next();
        if (err == null) continue;
        r.println(smeId + ",errc-" + err.errcode + "," + err.counter);
        totalCounters.incError(err.errcode, err.counter);
      }
    }
    r.close();

    if (writeTotals) {
      SmeIdCountersSet set = totalCounters;
      String smeId = totalCounters.smeid;
      r.println(smeId + ",accepted," + set.accepted);
      r.println(smeId + ",rejected," + set.rejected);
      r.println(smeId + ",delivered," + set.delivered);
      r.println(smeId + ",failed," + set.failed);
      r.println(smeId + ",rescheduled," + set.rescheduled);
      r.println(smeId + ",temporal," + set.temporal);
      r.println(smeId + ",peak_i," + set.peak_i);
      r.println(smeId + ",peak_o," + set.peak_o);
      for (Iterator j = set.getErrors().iterator(); j.hasNext();) {
        ErrorCounterSet err = (ErrorCounterSet) j.next();
        if (err == null) continue;
        r.println(smeId + ",errc-" + err.errcode + "," + err.counter);
      }
    }
  }

  private TreeMap getStatQueryDirs() throws Exception {
    String[] dirNames = srcDir.list();
    if (dirNames == null || dirNames.length == 0)
      throw new Exception("No stat directories at path '" + srcDir.getAbsolutePath() + "'");

    dateDirFormat.setCalendar(calendar);
    dateDirFileFormat.setCalendar(calendar);
    Date tillQueryDirTime = tillDate;
    Date tillQueryFileTime = tillDate;
    Date fromQueryDirTime = null;
    Date fromQueryFileTime = null;
    if (fromDate != null) {
      calendar.setTime(fromDate);
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
      }
      catch (ParseException exc) {
        System.out.println("Warn: " + exc.getMessage());
        continue;
      }

      if (fromQueryDirTime != null && dirDate.getTime() < fromQueryDirTime.getTime()) continue;
      if (tillQueryDirTime != null && dirDate.getTime() > tillQueryDirTime.getTime()) continue;

      File dirNameFile = new File(srcDir, dirName);
      File[] dirFiles = dirNameFile.listFiles();
      //logger.debug("Stat path: dir '" + dirNameFile.getAbsolutePath() + "'");
      if (dirFiles == null || dirFiles.length == 0) continue;

      for (int j = 0; j < dirFiles.length; j++) {
        String fileName = dirFiles[j].getName();
        if (fileName == null || fileName.length() <= 0 ||
                !fileName.toLowerCase().endsWith(DATE_FILE_EXTENSION)) continue;

        Date fileDate;
        try {
          fileDate = dateDirFileFormat.parse(dirName + File.separatorChar + fileName);
        }
        catch (ParseException exc) {
          System.out.println("Warn: " + exc.getMessage());
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

  private static void readBuffer(InputStream is, byte buffer[], int size) throws IOException {
    int read = 0;
    while (read < size) {
      int result = is.read(buffer, read, size - read);
      if (result < 0) throw new EOFException("Failed to read " + size + " bytes, read failed at " + read);
      read += result;
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


  public static void main(String[] args) {
    ExportStat stat = null;
    try {
      stat = new ExportStat(args[0], args[1], args[2], args.length > 3? args[3] : null, args.length > 4 ? args[4] : null, args.length > 5 ? args[5] : null);
    }
    catch (Exception e) {
      e.printStackTrace();
      System.out.println("java ExportStat srcDir destDir [ExportDate] [UseStartDateInHeader] [WriteTotals]");
      System.out.println("ExportDate in yyyy-MM-dd-HH format");
      System.out.println("UseStartDateInHeader=true/false");
      System.out.println("WriteTotals=true/false");
      return;
    }

    try {
      if (stat != null) stat.process();
      else {
        System.out.println("ExportStat has not been initialized");
        return;
      }
    }
    catch (Exception e) {
      e.printStackTrace();
      return;
    }

    System.out.println("done.");
  }
}
