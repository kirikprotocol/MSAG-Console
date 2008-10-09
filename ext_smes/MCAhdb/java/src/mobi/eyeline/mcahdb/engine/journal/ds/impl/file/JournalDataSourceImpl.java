package mobi.eyeline.mcahdb.engine.journal.ds.impl.file;

import mobi.eyeline.mcahdb.engine.journal.ds.JournalDataSource;
import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.journal.ds.Journal;
import mobi.eyeline.mcahdb.engine.journal.ds.JournalEvent;
import mobi.eyeline.mcahdb.engine.journal.JournalsProcessor;

import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * User: artem
 * Date: 31.07.2008
 */

public class JournalDataSourceImpl implements JournalDataSource {

  private final SimpleDateFormat archiveDateDirFormat = new SimpleDateFormat("yyMM/dd");

  private final File journalsDir;
  private final FileFilter journalsFilter;
  private final File archivesDir;

  public JournalDataSourceImpl(File journalsDir, File archivesDir) throws DataSourceException {
    this.journalsDir = journalsDir;
    this.archivesDir = archivesDir;

    if (!journalsDir.exists() && !journalsDir.mkdirs())
        throw new DataSourceException("Can't create journals dir: " + journalsDir.getAbsolutePath());

    if (!archivesDir.exists() && !archivesDir.mkdirs())
      throw new DataSourceException("Can't create archives dir: " + archivesDir.getAbsolutePath());

    this.journalsFilter = new FileFilter() {
      public boolean accept(File pathname) {
        return !pathname.isDirectory() && pathname.getName().matches(".*\\.csv");
      }
    };
  }

  public JournalDataSourceImpl(JournalsProcessor.Config config) throws DataSourceException {
    this(new File(config.getJournalsStoreDir()), new File(config.getJournalsArchivesDir()));
  }

  public void getJournals(Collection<Journal> result) throws DataSourceException {
    File[] files = journalsDir.listFiles(journalsFilter);
    if (files == null || files.length == 0)
      return;

    for (File f : files)
      result.add(new JournalImpl(f));    
  }

  public Journal getJournal(String journalName) throws DataSourceException {
    File journalFile = new File(journalsDir, journalName);
    if (!journalFile.exists())
      throw new DataSourceException("Journal file does not exists: " + journalName);

    return new JournalImpl(journalFile);
  }

  public void removeJournal(String journalName) throws DataSourceException {
    File journalFile = new File(journalsDir, journalName);
    if (!journalFile.exists())
      throw new DataSourceException("Journal file does not exists: " + journalName);

    JournalImpl jimpl = new JournalImpl(journalFile);

    File storeDir = new File(archivesDir, archiveDateDirFormat.format(jimpl.getStartDate()));
    if (!storeDir.exists() && !storeDir.mkdirs())
      throw new DataSourceException("Can't create dir " + storeDir.getAbsolutePath());
    
    if (!journalFile.renameTo(new File(storeDir,  journalName)))
      throw new DataSourceException("Can't remove journal file : " + journalName);
  }



  private static class JournalImpl implements Journal {

    private final SimpleDateFormat df = new SimpleDateFormat("yyyyMMdd HH:mm:ss.SSS");
    private final SimpleDateFormat startdf = new SimpleDateFormat("yyyyMMddHHmmss");

    private BufferedReader is;
    private final String journalName;
    private final File journalFile;
    private final Date startDate;

    private int lineNumber = -1;


    private JournalImpl(File journalFile) throws DataSourceException {
      this.journalName = journalFile.getName();
      this.journalFile = journalFile;

      int i = this.journalName.indexOf('.');
      int j = this.journalName.indexOf('.', i+1);
      if (i == j || i < 0 || j < 0)
        throw new DataSourceException("Illegal journal file name: " + journalName);

      try {
        String dateString = journalName.substring(i+1, j);
        this.startDate = startdf.parse(dateString);
      } catch (ParseException e) {
        throw new DataSourceException("Illegal journal file name: " + journalName);
      }
    }

    private static String[] parseLine(String str, int count) throws DataSourceException {

      String[] result = new String[count];

      int i=0, j=0, k=0;

      while ((i = str.indexOf(',', j)) >=0 && k < count - 1) {
        String st = str.substring(j,i);
        result[k++] = st;
        j = i + 1;
      }

      if (k != count -1)
        throw new DataSourceException("Invalid line, expected count=" + count + ": " + str);

      result[count-1] = str.substring(j);

      return result;
    }

    private JournalEvent readMissedCall(String[] st) throws DataSourceException {
      JournalEvent e = new JournalEvent();
      e.setType(JournalEvent.Type.MissedCall);

      try {
        e.setDate(df.parse(st[0]));
      } catch (ParseException e1) {
        throw new DataSourceException("Invalid date format");
      }

      e.setCaller(st[1]);
      e.setCalled(st[2]);
      e.setCalledProfileNotify(st[3].charAt(0) == '1');
      e.setCallerProfileWantNotifyMe(st[4].charAt(0) == '1');

      return e;
    }

    private JournalEvent readMissedCallAlert(String[] st) throws DataSourceException {
      JournalEvent e = new JournalEvent();
      e.setType(JournalEvent.Type.MissedCallAlert);

      try {
        e.setDate(df.parse(st[0]));
      } catch (ParseException e1) {
        throw new DataSourceException("Invalid date format");
      }

      e.setCaller(st[1]);
      e.setCalled(st[2]);
      e.setCalledProfileNotify(st[3].charAt(0) == '1');
      e.setCallerProfileWantNotifyMe(st[4].charAt(0) == '1');

      return e;
    }

    private JournalEvent readMissedCallAlertFail(String[] st) throws DataSourceException {
      JournalEvent e = new JournalEvent();
      e.setType(JournalEvent.Type.MissedCallAlertFail);

      try {
        e.setDate(df.parse(st[0]));
      } catch (ParseException e1) {
        throw new DataSourceException("Invalid date format");
      }

      e.setCaller(st[1]);
      e.setCalled(st[2]);

      return e;
    }

    private JournalEvent readMissedCallRemove(String[] st) throws DataSourceException {
      JournalEvent e = new JournalEvent();
      e.setType(JournalEvent.Type.MissedCallRemove);

      try {
        e.setDate(df.parse(st[0]));
      } catch (ParseException e1) {
        throw new DataSourceException("Invalid date format");
      }

      e.setCaller(st[1]);
      e.setCalled(st[2]);

      return e;
    }

    private JournalEvent readProfileChanged(String[] st) throws DataSourceException {
      JournalEvent e = new JournalEvent();
      e.setType(JournalEvent.Type.ProfileChaged);

      try {
        e.setDate(df.parse(st[0]));
      } catch (ParseException e1) {
        throw new DataSourceException("Invalid date format");
      }

      e.setCalled(st[1]);
      e.setCallerProfileWantNotifyMe(st[2].charAt(0) == '1');

      return e;
    }

    public Date getStartDate() {
      return startDate;
    }

    public String getName() {
      return journalName;
    }

    public JournalEvent nextEvent() throws DataSourceException {
      try {
        String str;
        if ((str = is.readLine()) != null) {
          lineNumber++;
          char type = str.charAt(0);
          switch (type) {
            case 'A': return readMissedCall(parseLine(str.substring(2), 5));
            case 'D': return readMissedCallAlert(parseLine(str.substring(2), 5));
            case 'F': return readMissedCallAlertFail(parseLine(str.substring(2), 3));
            case 'E': return readMissedCallRemove(parseLine(str.substring(2), 3));
            case 'P': return readProfileChanged(parseLine(str.substring(2), 3));
            default:return null;
          }
        } else
          return null;        

      } catch (IOException e) {
        throw new DataSourceException("Read journal error: ", e);
      } catch (DataSourceException ex) {
        throw new DataSourceException(ex.getMessage() + " in journal " + journalName + ", line=" + lineNumber, ex);
      }
    }

    public void open() throws DataSourceException {
      try {
        this.is = new BufferedReader(new FileReader(journalFile));
      } catch (FileNotFoundException e) {
        throw new DataSourceException("Can't open journal: " + journalFile.getAbsolutePath(), e);
      }
    }

    public void close() {
      try {
        is.close();
      } catch (IOException e) {        
      }
    }
  }

  public static void main(String ... args) throws DataSourceException {
    String [] r = JournalImpl.parseLine("A,20080918 12:34:05.646,255,+79139030,1", 6);
    for (String s : r)
      System.out.println(s);
  }
}
