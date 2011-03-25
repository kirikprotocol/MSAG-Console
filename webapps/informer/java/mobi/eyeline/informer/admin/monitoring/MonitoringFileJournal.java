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
  public void visit(MonitoringFilter filter, Visitor v) throws AdminException {
    try {
      Collection<File> files = getFiles(filter);
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
              MonitoringEvent record = convert(line); //todo Отсюда может вылететь IndexOutOfBoundsException, который не перехватится.
              if (!filter.accept(record)) {
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

  protected static MonitoringEvent convert(String ss) {

    List<String> es = StringEncoderDecoder.csvSplit(sepChar,ss);

    // todo В одном из этих методов может возникнуть IndexOutOfBoundsException, который не будет перехвачен в методе visit
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

    if(es.size()>5) { //todo эту проверочку выше поставить надо...
      for(int i=5; i<es.size();i++) {
        String[] ps = es.get(i).split("=",2);
        e.setProperty(ps[0], ps[1]);
      }
    }

    return e;
  }

  private List<File> getFiles(final MonitoringFilter filter) throws ParseException {

    SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHH");

    Date fromDate = filter == null || filter.getStartDate() == null ? null : df.parse(df.format(filter.getStartDate()));
    Date toDate = filter == null || filter.getEndDate() == null ? null : df.parse(df.format(filter.getEndDate()));

    SimpleDateFormat ydf = new SimpleDateFormat("yyyy");
    SimpleDateFormat mdf = new SimpleDateFormat("yyyyMM");
    SimpleDateFormat ddf = new SimpleDateFormat("yyyyMMdd");
    SimpleDateFormat hdf = new SimpleDateFormat("yyyyMMddHH");

    List<File> results = new LinkedList<File>();

    long showFrom = System.currentTimeMillis() - SHOW_PERIOD; //todo Зачем это захардкоженое ограничение? Может так:
                                                              //todo if (fromDate == null) fromDate = new Date(System.currentTimeMillis() - SHOW_PERIOD)
                                                              //todo Но в идеале это условие надо перенести в Controller, и убрать из модели.

    // todo 4 вложенных цикла(!). Надо упростить (например, разбить на методы).
    for (File y : dir.listFiles()) {
      Date yd = ydf.parse(y.getName());
      if (toDate != null && yd.after(ydf.parse(ydf.format(toDate)))) {
        continue;
      }
      if (fromDate != null && yd.before(ydf.parse(ydf.format(fromDate)))) {
        continue;
      }
      for (File m : y.listFiles()) {
        Date md = mdf.parse(y.getName() + m.getName());
        if (toDate != null && md.after(mdf.parse(mdf.format(toDate)))) {
          continue;
        }
        if (fromDate != null && md.before(mdf.parse(mdf.format(fromDate)))) {
          continue;
        }
        for (File d : m.listFiles()) {
          Date dd = ddf.parse(y.getName() + m.getName() + d.getName());
          if (toDate != null && dd.after(ddf.parse(ddf.format(toDate)))) {
            continue;
          }
          if (fromDate != null && dd.before(ddf.parse(ddf.format(fromDate)))) {
            continue;
          }
          for (File h : d.listFiles()) {
            Date hd = hdf.parse(y.getName() + m.getName() + d.getName() + h.getName().substring(0, h.getName().indexOf(".")));

            if (hd.getTime() < showFrom) {
              continue;
            }

            if (toDate != null && hd.after(hdf.parse(hdf.format(toDate)))) {
              continue;
            }
            if (fromDate != null && hd.before(hdf.parse(hdf.format(fromDate)))) {
              continue;
            }
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
