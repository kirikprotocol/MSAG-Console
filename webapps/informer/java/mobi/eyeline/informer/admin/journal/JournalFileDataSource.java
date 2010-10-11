package mobi.eyeline.informer.admin.journal;

import com.eyeline.utils.FileUtils;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Файловый сторадж журнала
 * Возвращает записи, созданные не более 1 недели назад
 * @author Aleksandr Khalitov
 */
class JournalFileDataSource implements JournalDataSource{

  private static final long SHOW_PERIOD = 7*24*60*60*1000;

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private static final Logger logger = Logger.getLogger(JournalFileDataSource.class);

  private final File journalDir;

  private final FileSystem fileSystem;

  public JournalFileDataSource(File journalDir, FileSystem fileSystem) throws AdminException{
    this.journalDir = journalDir;
    this.fileSystem = fileSystem;
    if(!journalDir.exists() && !journalDir.mkdirs()) {
      throw new IllegalArgumentException("Can't create dir: "+journalDir.getAbsolutePath());
    }

    try{
      long time = System.currentTimeMillis();
      Collection<File> files = getFiles(null);
      for(File f : files) {
        if(FileUtils.truncateFile(f, System.getProperty("line.separator").getBytes()[0], 30)) {
          if(logger.isInfoEnabled()) {
            logger.info("Journal file was repaired: "+f.getAbsolutePath());
          }
        }
      }
      if(logger.isInfoEnabled()) {
        logger.info("Journal is loaded: "+(System.currentTimeMillis() - time)+" millis");
      }
    }catch (IOException e){
      logger.error(e,e);
      throw new IllegalArgumentException(e);
    }catch (ParseException e){
      logger.error(e,e);
      throw new IllegalArgumentException(e);
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void addRecords(JournalRecord ... records) throws AdminException {
    if(records == null || records.length == 0) {
      return;
    }
    Date date = new Date();
    SimpleDateFormat sdf = new SimpleDateFormat("yyyy"+File.separatorChar+"MM"+File.separator+"dd");
    File d = new File(journalDir, sdf.format(date));
    if(!d.exists() && !d.mkdirs()) {
      throw new JournalException("cant_add_record");
    }
    File file = new File(d, new SimpleDateFormat("HH").format(date) + ".csv");     

    long dateL = date.getTime();

    PrintWriter writer = null;
    try{
      lock.writeLock().lock();
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fileSystem.getOutputStream(file, true))));
      for(JournalRecord record : records) {
        record.setTime(dateL);
        write(record, writer);
      }
    }finally {
      if(writer != null) {
        try{
          writer.close();
        }catch (Exception ex) {}
      }
      lock.writeLock().unlock();
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void visit(JournalFilter filter, JournalVisitor v) throws AdminException {
    try{
      Collection<File> files = getFiles(filter);
      try{
        lock.readLock().lock();
        for(File f : files) {
          BufferedReader reader = null;
          try{
            reader = new BufferedReader(new InputStreamReader(fileSystem.getInputStream(f)));
            String line;
            while((line = reader.readLine()) != null) {
              if(line.length() == 0) {
                continue;                   // may happens in repaired file
              }
              JournalRecord record = convert(line);
              if(!filter.accept(record)) {
                continue;
              }
              if(!v.visit(record)) {
                return;
              }
            }
          }finally {
            if(reader != null) {
              try{
                reader.close();
              }catch (IOException e){}
            }
          }
        }
      }finally {
        lock.readLock().unlock();
      }
    }catch (IOException e){
      logger.error(e,e);
      throw new JournalException("cant_read_journal", e);
    }catch (ParseException e) {
      logger.error(e,e);
      throw new JournalException("cant_read_journal", e);
    }catch (NumberFormatException e) {
      logger.error(e,e);
      throw new JournalException("cant_read_journal", e);
    }
  }

  private static final char sepChar = '|';
  private static final String sep = new String(new char[]{sepChar});

  private static final String NULL = "null";

  protected static void writeStr(String str, PrintWriter w) {
    if (str == null || str.length() == 0)
      w.write(NULL);
    else {
      for (int i = 0; i < str.length(); i++) {
        char ch = str.charAt(i);
        if (ch == sepChar)
          w.print('\\');
        w.print(ch);
        if (ch == sepChar)
          w.print(' ');
      }
    }
  }

  protected void write(JournalRecord r, PrintWriter w) {
    w.write(Long.toString(r.getTime()));
    w.write(sepChar);
    writeStr(r.getUser(),w);
    w.write(sepChar);
    writeStr(r.getSubject().getKey(), w);
    w.write(sepChar);
    w.write(r.getType() == null ? null : r.getType().toString());
    w.write(sepChar);
    writeStr(r.getDescriptionKey(), w);
    if(r.getDescriptionArgs() != null) {
      for(String a : r.getDescriptionArgs()) {
        w.write(sepChar);
        writeStr(a, w);
      }
    }
    w.println();
  }

  protected static JournalRecord convert(String ss) {
    StringTokenizer t = new StringTokenizer(ss, sep);
    String sTemp;
    long time = Long.parseLong(t.nextToken());
    String user = (sTemp = t.nextToken()).equals(NULL) ? null : sTemp;
    if (user != null) {
      while (user.indexOf("\\") != -1) {
        user = user.replace("\\", sep) + t.nextToken().substring(1);
      }
    }
    String subjectKey = (sTemp = t.nextToken()).equals(NULL) ? null : sTemp;
    if (subjectKey != null) {
      while (subjectKey.indexOf("\\") != -1) {
        subjectKey = subjectKey.replace("\\", sep) + t.nextToken().substring(1);
      }
    }
    JournalRecord.Type type = (sTemp = t.nextToken()).equals(NULL) ? null : JournalRecord.Type.valueOf(sTemp);

    String descrKey = (sTemp = t.nextToken()).equals(NULL) ? null : sTemp;
    if (descrKey != null) {
      while (descrKey.indexOf("\\") != -1) {
        descrKey = descrKey.replace("\\", sep) + t.nextToken().substring(1);
      }
    }

    List<String> args = new LinkedList<String>();
    while(t.hasMoreTokens()) {
      String arg = t.nextToken();
      if (arg != null) {
        while (arg.indexOf("\\") != -1) {
          arg = arg.replace("\\", sep) + t.nextToken().substring(1);
        }
        args.add(arg);
      }
    }
    JournalRecord record = new JournalRecord(type);
    record.setDescription(descrKey, args.toArray(new String[args.size()]));
    record.setSubject(Subject.getByKey(subjectKey));
    record.setUser(user);
    record.setTime(time);
    return record;
  }

  private List<File> getFiles(final JournalFilter filter) throws ParseException {

    SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHH");

    Date fromDate = filter == null || filter.getStartDate() == null ? null : df.parse(df.format(filter.getStartDate()));
    Date toDate = filter == null || filter.getEndDate() == null ? null : df.parse(df.format(filter.getEndDate()));

    SimpleDateFormat ydf = new SimpleDateFormat("yyyy");
    SimpleDateFormat mdf = new SimpleDateFormat("yyyyMM");
    SimpleDateFormat ddf = new SimpleDateFormat("yyyyMMdd");
    SimpleDateFormat hdf = new SimpleDateFormat("yyyyMMddHH");

    List<File> results = new LinkedList<File>();

    long showFrom = System.currentTimeMillis() - SHOW_PERIOD;

    for(File y : journalDir.listFiles()) {
      Date yd = ydf.parse(y.getName());
      if(toDate != null && yd.after(ydf.parse(ydf.format(toDate))))  {
        continue;
      }
      if(fromDate != null && yd.before(ydf.parse(ydf.format(fromDate))))  {
        continue;
      }
      for(File m : y.listFiles()) {
        Date md = mdf.parse(y.getName()+m.getName());
        if(toDate != null && md.after(mdf.parse(mdf.format(toDate))))  {
          continue;
        }
        if(fromDate != null && md.before(mdf.parse(mdf.format(fromDate)))) {
          continue;
        }
        for(File d : m.listFiles()) {
          Date dd = ddf.parse(y.getName()+m.getName()+d.getName());
          if(toDate != null && dd.after(ddf.parse(ddf.format(toDate))))  {
            continue;
          }
          if(fromDate != null && dd.before(ddf.parse(ddf.format(fromDate))))  {
            continue;
          }
          for(File h : d.listFiles()) {
            Date hd = hdf.parse(y.getName()+m.getName()+d.getName()+h.getName().substring(0, h.getName().indexOf(".")));

            if(hd.getTime() < showFrom) {
              continue;
            }

            if(toDate != null && hd.after(hdf.parse(hdf.format(toDate))))  {
              continue;
            }
            if(fromDate != null && hd.before(hdf.parse(hdf.format(fromDate))))  {
              continue;
            }
            results.add(h);
          }
        }
      }
    }
    if(!results.isEmpty()) {
      Collections.sort(results);
    }
    return results;
  }
}
