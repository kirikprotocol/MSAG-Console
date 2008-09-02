package mobi.eyeline.mcahdb.engine.journal.ds.impl.file;

import mobi.eyeline.mcahdb.engine.journal.ds.JournalDataSource;
import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.journal.ds.Journal;
import mobi.eyeline.mcahdb.engine.journal.ds.JournalEvent;
import mobi.eyeline.mcahdb.engine.journal.JournalsProcessor;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * User: artem
 * Date: 31.07.2008
 */

public class JournalDataSourceImpl implements JournalDataSource {

  private final static SimpleDateFormat archiveDateDirFormat = new SimpleDateFormat("yyMM/dd");

  private final File journalsDir;
  private final FileFilter journalsFilter;
  private final File archivesDir;

  public JournalDataSourceImpl(JournalsProcessor.Config config) throws DataSourceException {
    this.journalsDir = new File(config.getJournalsStoreDir());
    this.archivesDir = new File(config.getJournalsArchivesDir());

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

    private static final SimpleDateFormat df = new SimpleDateFormat("yyyyMMdd HH:mm:ss.SSS");
    private static final SimpleDateFormat startdf = new SimpleDateFormat("yyyyMMddHHmmss");

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

    private static JournalEvent readMissedCall(StringTokenizer st) throws DataSourceException {
      JournalEvent e = new JournalEvent();
      e.setType(JournalEvent.Type.MissedCall);

      if (st.countTokens() != 5)
        throw new DataSourceException("Invalid missed call record");

      try {
        e.setDate(df.parse(st.nextToken()));
      } catch (ParseException e1) {
        throw new DataSourceException("Invalid date format");
      }

      e.setCaller(st.nextToken());
      e.setCalled(st.nextToken());
      e.setCalledProfileNotify(st.nextToken().equals("1"));
      e.setCallerProfileWantNotifyMe(st.nextToken().equals("1"));

      return e;
    }

    private static JournalEvent readMissedCallAlert(StringTokenizer st) throws DataSourceException {
      JournalEvent e = new JournalEvent();
      e.setType(JournalEvent.Type.MissedCallAlert);

      if (st.countTokens() != 5)
        throw new DataSourceException("Invalid missed call record");

      try {
        e.setDate(df.parse(st.nextToken()));
      } catch (ParseException e1) {
        throw new DataSourceException("Invalid date format");
      }

      e.setCaller(st.nextToken());
      e.setCalled(st.nextToken());
      e.setCalledProfileNotify(st.nextToken().equals("1"));
      e.setCallerProfileWantNotifyMe(st.nextToken().equals("1"));

      return e;
    }

    private static JournalEvent readMissedCallAlertFail(StringTokenizer st) throws DataSourceException {
      JournalEvent e = new JournalEvent();
      e.setType(JournalEvent.Type.MissedCallAlertFail);

      if (st.countTokens() != 3)
        throw new DataSourceException("Invalid missed call record");

      try {
        e.setDate(df.parse(st.nextToken()));
      } catch (ParseException e1) {
        throw new DataSourceException("Invalid date format");
      }

      e.setCaller(st.nextToken());
      e.setCalled(st.nextToken());

      return e;
    }

    private static JournalEvent readMissedCallRemove(StringTokenizer st) throws DataSourceException {
      JournalEvent e = new JournalEvent();
      e.setType(JournalEvent.Type.MissedCallRemove);

      if (st.countTokens() != 3)
        throw new DataSourceException("Invalid missed call record");

      try {
        e.setDate(df.parse(st.nextToken()));
      } catch (ParseException e1) {
        throw new DataSourceException("Invalid date format");
      }

      e.setCaller(st.nextToken());
      e.setCalled(st.nextToken());

      return e;
    }

    private static JournalEvent readProfileChanged(StringTokenizer st) throws DataSourceException {
      JournalEvent e = new JournalEvent();
      e.setType(JournalEvent.Type.ProfileChaged);

      if (st.countTokens() != 3)
        throw new DataSourceException("Invalid missed call record");

      try {
        e.setDate(df.parse(st.nextToken()));
      } catch (ParseException e1) {
        throw new DataSourceException("Invalid date format");
      }

      e.setCalled(st.nextToken());
      e.setCallerProfileWantNotifyMe(st.nextToken().equals("1"));

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
        while ((str = is.readLine()) != null) {
          lineNumber++;
          StringTokenizer st = new StringTokenizer(str, ",");

          final String type = st.nextToken();
          if (type.equals("A"))
            return readMissedCall(st);
          else if (type.equals("D"))
            return readMissedCallAlert(st);
          else if (type.equals("F"))
            return readMissedCallAlertFail(st);
          else if (type.equals("E"))
            return readMissedCallRemove(st);
          else if (type.equals("P"))
            return readProfileChanged(st);
        }

        return null;
        
      } catch (IOException e) {
        throw new DataSourceException("Read journal error: ", e);
      } catch (DataSourceException ex) {
        throw new DataSourceException(ex.getMessage() + " in journal " + journalName + ", line=" + lineNumber);
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
}
