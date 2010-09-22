package mobi.eyeline.informer.web.journal;

import com.eyeline.utils.FileUtils;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.web.InitException;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Файловый сторадж журнала
 * @author Aleksandr Khalitov
 */
public class JournalFileDataSource implements JournalDataSource{


  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private static final Logger logger = Logger.getLogger(JournalFileDataSource.class);

  private final File journalDir;

  private final FileSystem fileSystem;

  public JournalFileDataSource(File journalDir, FileSystem fileSystem) throws InitException{
    this.journalDir = journalDir;
    this.fileSystem = fileSystem;
    if(!journalDir.exists() && !journalDir.mkdirs()) {
      throw new InitException("Can't create dir: "+journalDir.getAbsolutePath());
    }

    try{
      long time = System.currentTimeMillis();
      Collection<File> files = getFiles(null);
      for(File f : files) {
        if(FileUtils.truncateFile(f, System.getProperty("line.separator").getBytes()[0], 30)) {
          if(logger.isInfoEnabled()) {
            logger.info("Journal file was repaired: "+f.getAbsolutePath());
          }
          System.out.println("File was repaired: "+f.getAbsolutePath());
        }
      }
      System.out.println("Journal is loaded: "+(System.currentTimeMillis() - time)+" millis");
    }catch (IOException e){
      logger.error(e,e);
      throw new InitException(e);
    }catch (ParseException e){
      logger.error(e,e);
      throw new InitException(e);
    }
  }

  public void addRecords(JournalRecord ... records) throws AdminException {
    Map<File, List<JournalRecord>> byFile = new HashMap<File, List<JournalRecord>>();
    for(JournalRecord record : records) {
      Date date = new Date(record.getTime());
      SimpleDateFormat sdf = new SimpleDateFormat("yyyy"+File.separatorChar+"MM"+File.separator+"dd");
      File d = new File(journalDir, sdf.format(date));
      File file = new File(d, new SimpleDateFormat("HH").format(date) + ".csv");
      List<JournalRecord> rs = byFile.get(file);
      if(rs == null) {
        rs = new LinkedList<JournalRecord>();
        byFile.put(file, rs);
      }
      rs.add(record);
    }
    for(Map.Entry<File, List<JournalRecord>> e : byFile.entrySet()) {
      File file = e.getKey();
      for(JournalRecord record : e.getValue()) {
        PrintWriter writer = null;
        try{
          lock.writeLock().lock();
          File parent = file.getParentFile();
          if(!parent.exists() && !parent.mkdirs()) {
            throw new JournalException("cant_add_record");
          }
          writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fileSystem.getOutputStream(file, true))));
          write(record, writer);
        }finally {
          if(writer != null) {
            try{
              writer.close();
            }catch (Exception ex) {}
          }
          lock.writeLock().unlock();
        }
      }
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void visit(JournalFilter filter, Visitor v) throws AdminException {
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
    writeStr(r.getSubjectKey(), w);
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
    record.setSubjectKey(subjectKey);
    record.setUser(user);
    record.setTime(time);
    return record;
  }

  private Collection<File> getFiles(final JournalFilter filter) throws ParseException {

    SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHH");

    Date fromDate = filter == null || filter.getStartDate() == null ? null : df.parse(df.format(filter.getStartDate()));
    Date toDate = filter == null || filter.getEndDate() == null ? null : df.parse(df.format(filter.getEndDate()));

    SimpleDateFormat ydf = new SimpleDateFormat("yyyy");
    SimpleDateFormat mdf = new SimpleDateFormat("yyyyMM");
    SimpleDateFormat ddf = new SimpleDateFormat("yyyyMMdd");
    SimpleDateFormat hdf = new SimpleDateFormat("yyyyMMddHH");

    Collection<File> results = new LinkedList<File>();

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

    return results;
  }
}
