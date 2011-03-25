package mobi.eyeline.informer.admin.monitoring;

import com.eyeline.utils.FileUtils;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * @author Aleksandr Khalitov
 */
public class MonitoringFileJournal implements MonitoringJournal {

  private static final long SHOW_PERIOD = 7 * 24 * 60 * 60 * 1000;

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private static final Logger logger = Logger.getLogger(MonitoringFileJournal.class);

  private final File dir;

  private final FileSystem fileSystem;

  public MonitoringFileJournal(File dir, FileSystem fileSystem) throws AdminException {
    this.dir = dir;
    this.fileSystem = fileSystem;
    if (!fileSystem.exists(dir)) {
      fileSystem.mkdirs(dir);
    }

    try {
      long time = System.currentTimeMillis();
      Collection<File> files = getFiles(null);
      for (File f : files) {
        if (FileUtils.truncateFile(f, System.getProperty("line.separator").getBytes()[0], 30)) {
          if (logger.isInfoEnabled()) {
            logger.info("Journal file was repaired: " + f.getAbsolutePath());
          }
        }
      }
      if (logger.isInfoEnabled()) {
        logger.info("Journal is loaded: " + (System.currentTimeMillis() - time) + " millis");
      }
    } catch (IOException e) {
      logger.error(e, e);
      throw new IllegalArgumentException(e);
    } catch (ParseException e) {
      logger.error(e, e);
      throw new IllegalArgumentException(e);
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void addEvents(MonitoringEvent... events) throws AdminException {
    if (events == null || events.length == 0) {
      return;
    }
    Date date = new Date();
    SimpleDateFormat sdf = new SimpleDateFormat("yyyy" + File.separatorChar + "MM" + File.separator + "dd");
    File d = new File(dir, sdf.format(date));
    if (!fileSystem.exists(d)) {
       fileSystem.mkdirs(d);
    }
    File file = new File(d, new SimpleDateFormat("HH").format(date) + ".csv");

    long dateL = date.getTime();

    PrintWriter writer = null;
    try {
      lock.writeLock().lock();
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fileSystem.getOutputStream(file, true))));
      for (MonitoringEvent event : events) {
        event.setTime(dateL);
        write(event, writer);
      }
    } catch (IOException e){
      logger.error(e,e);
      throw new MonitoringException("cant_add_record");
    } finally {
      if (writer != null) {
        try {
          writer.close();
        } catch (Exception ex) {
        }
      }
      lock.writeLock().unlock();
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void visit(MonitoringEventsFilter eventsFilter, Visitor v) throws AdminException {
    try {
      Collection<File> files = getFiles(eventsFilter);
      try {
        lock.readLock().lock();
        for (File f : files) {
          BufferedReader reader = null;
          try {
            reader = new BufferedReader(new InputStreamReader(fileSystem.getInputStream(f)));
            String line;
            while ((line = reader.readLine()) != null) {
              if (line.length() == 0) {
                continue;                   // may happens in repaired file
              }
              MonitoringEvent record = parse(line);

              if (record == null || !eventsFilter.accept(record)) {
                continue;
              }
              if (!v.visit(record)) {
                return;
              }
            }
          } finally {
            if (reader != null) {
              try {
                reader.close();
              } catch (IOException e) {
              }
            }
          }
        }
      } finally {
        lock.readLock().unlock();
      }
    } catch (IOException e) {
      logger.error(e, e);
      throw new MonitoringException("cant_read_journal");
    } catch (ParseException e) {
      logger.error(e, e);
      throw new MonitoringException("cant_read_journal");
    } catch (NumberFormatException e) {
      logger.error(e, e);
      throw new MonitoringException("cant_read_journal");
    }
  }

  private static final char sepChar = ';';

  protected void write(MonitoringEvent ev, PrintWriter w) throws IOException {
    Map<String, String> props = ev.getProps();
    List<Object> array = new ArrayList<Object>(5+props.size());
    array.add(ev.getTime());
    array.add(ev.getSource());
    array.add(ev.getAlarmId());
    array.add(ev.getSeverity());
    array.add(ev.getText());

    for(Map.Entry<String, String> e : props.entrySet()) {
      array.add(e.getKey()+"="+e.getValue());
    }
    StringEncoderDecoder.toCSV(sepChar, w, array.toArray());
    w.println();
  }

  protected static MonitoringEvent parse(String ss) {
    try{
      List<String> es = StringEncoderDecoder.csvSplit(sepChar,ss);

      if(es.size()<4) {
        logger.error("Can't parse line: "+ss);
        return null;
      }
      String time = es.get(0);
      String source = es.get(1);
      String alarmId = es.get(2);
      String severity = es.get(3);
      String text = es.get(4);

      MonitoringEvent e = new MonitoringEvent(alarmId);
      e.setTime(Long.parseLong(time));
      e.setSource(MBean.Source.valueOf(source));
      e.setSeverity(MonitoringEvent.Severity.valueOf(severity));
      e.setText(text);

      if(es.size()>5) {
        for(int i=5; i<es.size();i++) {
          String[] ps = es.get(i).split("=",2);
          e.setProperty(ps[0], ps[1]);
        }
      }

      return e;
    } catch (Exception e){
      logger.error(e,e);
      return null;
    }
  }

  private List<File> getYearsDir(File parent, Date fromDate, Date toDate) throws ParseException {
    List<File> result = new LinkedList<File>();
    SimpleDateFormat ydf = new SimpleDateFormat("yyyy");
    for (File y : parent.listFiles()) {
      Date yd = ydf.parse(y.getName());
      if (toDate != null && yd.after(ydf.parse(ydf.format(toDate)))) {
        continue;
      }
      if (fromDate != null && yd.before(ydf.parse(ydf.format(fromDate)))) {
        continue;
      }
      result.add(y);
    }
    return result;
  }

  private List<File> getMonthsDir(File y, Date fromDate, Date toDate) throws ParseException {
    List<File> result = new LinkedList<File>();
    SimpleDateFormat mdf = new SimpleDateFormat("yyyyMM");
    for (File m : y.listFiles()) {
      Date md = mdf.parse(y.getName() + m.getName());
      if (toDate != null && md.after(mdf.parse(mdf.format(toDate)))) {
        continue;
      }
      if (fromDate != null && md.before(mdf.parse(mdf.format(fromDate)))) {
        continue;
      }
      result.add(m);
    }
    return result;
  }

  private List<File> getDaysDir(File y, File m, Date fromDate, Date toDate) throws ParseException {
    List<File> result = new LinkedList<File>();
    SimpleDateFormat ddf = new SimpleDateFormat("yyyyMMdd");
    for (File d : m.listFiles()) {
      Date dd = ddf.parse(y.getName() + m.getName() + d.getName());
      if (toDate != null && dd.after(ddf.parse(ddf.format(toDate)))) {
        continue;
      }
      if (fromDate != null && dd.before(ddf.parse(ddf.format(fromDate)))) {
        continue;
      }
      result.add(d);
    }
    return result;
  }

  private List<File> getHoursDir(File y, File m, File d, Date fromDate, Date toDate) throws ParseException {
    List<File> result = new LinkedList<File>();
    SimpleDateFormat hdf = new SimpleDateFormat("yyyyMMddHH");

    for (File h : d.listFiles()) {
      Date hd = hdf.parse(y.getName() + m.getName() + d.getName() + h.getName().substring(0, h.getName().indexOf(".")));

      if (toDate != null && hd.after(hdf.parse(hdf.format(toDate)))) {
        continue;
      }
      if (fromDate != null && hd.before(hdf.parse(hdf.format(fromDate)))) {
        continue;
      }
      result.add(h);
    }
    return result;
  }

  private List<File> getFiles(final MonitoringEventsFilter eventsFilter) throws ParseException {

    SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHH");

    Date fromDate = eventsFilter == null || eventsFilter.getStartDate() == null ? null : df.parse(df.format(eventsFilter.getStartDate()));
    Date toDate = eventsFilter == null || eventsFilter.getEndDate() == null ? null : df.parse(df.format(eventsFilter.getEndDate()));

    List<File> results = new LinkedList<File>();

    for(File y : getYearsDir(dir, fromDate, toDate)) {
      for(File m : getMonthsDir(y, fromDate, toDate)) {
        for(File d : getDaysDir(y, m ,fromDate, toDate)) {
          for(File h: getHoursDir(y, m, d, fromDate, toDate)) {
            results.add(h);
          }
        }
      }
    }

    if (!results.isEmpty()) {
      Collections.sort(results);
    }
    return results;
  }
}
