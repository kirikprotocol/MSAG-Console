package mobi.eyeline.mcahdb.engine.ds.impl.file;

import mobi.eyeline.mcahdb.engine.ds.JournalDataSource;
import mobi.eyeline.mcahdb.engine.ds.DataSourceException;
import mobi.eyeline.mcahdb.engine.ds.Journal;
import mobi.eyeline.mcahdb.engine.ds.Event;
import mobi.eyeline.mcahdb.engine.ds.impl.EventImpl;
import mobi.eyeline.mcahdb.GlobalConfig;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * User: artem
 * Date: 31.07.2008
 */

public class JournalDataSourceImpl implements JournalDataSource {

  private final File journalsDir;
  private final FileFilter journalsFilter;
  private final File archivesDir;

  public JournalDataSourceImpl(Config config) throws DataSourceException {
    this.journalsDir = new File(config.getJournalsStoreDir());
    this.archivesDir = new File(config.getJournalsArchivesDir());

    if (!journalsDir.exists())
      throw new DataSourceException("Journals dir does not exists: " + journalsDir.getAbsolutePath());
    if (!archivesDir.exists())
      throw new DataSourceException("Archives dir does not exists: " + archivesDir.getAbsolutePath());

    this.journalsFilter = new FileFilter() {
      public boolean accept(File pathname) {
        return !pathname.isDirectory() && pathname.getName().matches(".*\\.csv");
      }
    };
  }

  public Collection<Journal> getJournals() throws DataSourceException {
    File[] files = journalsDir.listFiles(journalsFilter);
    if (files == null || files.length == 0)
      return Collections.emptyList();

    final Collection<Journal> result = new ArrayList<Journal>(files.length);
    for (File f : files)
      result.add(new JournalImpl(f));
    return result;
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

    if (!journalFile.renameTo(new File(archivesDir, journalName)))
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

    private static Event readMissedCall(StringTokenizer st) throws DataSourceException {
      EventImpl e = new EventImpl();
      e.setType(Event.Type.MissedCall);

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

    private static Event readMissedCallAlert(StringTokenizer st) throws DataSourceException {
      EventImpl e = new EventImpl();
      e.setType(Event.Type.MissedCallAlert);

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

    private static Event readMissedCallAlertFail(StringTokenizer st) throws DataSourceException {
      EventImpl e = new EventImpl();
      e.setType(Event.Type.MissedCallAlertFail);

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

    public Date getStartDate() {
      return startDate;
    }

    public String getName() {
      return journalName;
    }

    public Event nextEvent() throws DataSourceException {
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


  public interface Config {
    public String getJournalsStoreDir();
    public String getJournalsArchivesDir();
  }
}
