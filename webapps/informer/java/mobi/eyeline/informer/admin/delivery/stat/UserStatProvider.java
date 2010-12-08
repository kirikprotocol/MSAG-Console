package mobi.eyeline.informer.admin.delivery.stat;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.CSVTokenizer;
import mobi.eyeline.informer.util.Functions;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class UserStatProvider extends StatEntityProvider{

  private final File baseDir;
  private final FileSystem fileSys;
  private final String filePathFormat;

  public UserStatProvider(File baseDir, FileSystem fileSys) {
    this(baseDir, fileSys, "yyyy.MM.dd");
  }

  UserStatProvider(File baseDir, FileSystem fileSys, String subDirNameFormat) {
    this.baseDir = baseDir;
    this.fileSys = fileSys;
    this.filePathFormat = subDirNameFormat + File.separatorChar + "'dlv'HH'.log'";
  }

  public void visitEntities(Date from, Date till, StatEntityProvider.EntityVisitor visitor) throws AdminException {
    Date fromDate = prepareDateForFilesLookup(from);
    Date tillDate = prepareDateForFilesLookup(till);

    List<StatFile> files = StatUtils.lookupFiles(baseDir, new SimpleDateFormat(filePathFormat), fromDate, tillDate);

    for (StatFile statFile : files) {
      if (!visitor.visit(
          new StatEntity(prepareDateForEntitiesView(statFile.getDate()),
              new File(baseDir, statFile.getFileName()).length()))) {
        break;
      }
    }
  }


  @Override
  public synchronized void dropEntities(Date from, Date till) throws AdminException {
    Date fromDate = prepareDateForFilesLookup(from);
    Date tillDate = prepareDateForFilesLookup(till);

    List<StatFile> files = StatUtils.lookupFiles(baseDir, new SimpleDateFormat(filePathFormat), fromDate, tillDate);

    Calendar c = Calendar.getInstance();
    c.setTime(Functions.convertTime(new Date(), LOCAL_TIMEZONE, STAT_TIMEZONE));
    c.set(Calendar.HOUR_OF_DAY, 0);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    Date today = c.getTime();

    Set<File> parents = new HashSet<File>();
    for (StatFile f : files) {
      if(f.getDate().before(today)) {
        File _f = new File(baseDir, f.getFileName());
        if (_f.exists()) {
          if (!_f.delete()) {
            logger.error("Can't remove file: " + _f.getAbsolutePath());
            throw new DeliveryException("internal_error");
          }
        }
        File _p = _f.getParentFile();
        if (_p != null) {
          parents.add(_f.getParentFile());
        }
      }
    }

    for(File _p : parents) {
      if(_p.exists() && _p.list().length == 0) {
        if(!_p.delete()) {
          logger.error("Can't remove file: "+_p.getAbsolutePath());
          throw new DeliveryException("internal_error");
        }
      }
    }
  }

  public void accept(UserStatFilter filter, UserStatVisitor visitor) throws AdminException {

    Date fromDate = prepareDateForFilesLookup(filter.getFromDate());
    Date tillDate = prepareDateForFilesLookup(filter.getTillDate());

    List<StatFile> files = StatUtils.lookupFiles(baseDir, new SimpleDateFormat(filePathFormat), fromDate, tillDate);
    int total = files.size();

    UserStatFilter convertedFilter = new UserStatFilter();
    convertedFilter.setUser(filter.getUser());
    if (filter.getFromDate() != null)
      convertedFilter.setFromDate(Functions.convertTime(filter.getFromDate(), LOCAL_TIMEZONE, STAT_TIMEZONE));
    if (filter.getTillDate() != null)
      convertedFilter.setTillDate(Functions.convertTime(filter.getTillDate(), LOCAL_TIMEZONE, STAT_TIMEZONE));

    try {
      for (int i=0; i<files.size(); i++)
        processFile(convertedFilter, visitor, total, i, files.get(i).getFileName());
    } catch (IOException e) {
      throw new DeliveryException("filesys.ioexception", e);
    }
  }

  public boolean processFile(UserStatFilter filter, UserStatVisitor visitor, int total, int current, String f) throws AdminException, IOException {

    BufferedReader reader = null;

    try {

      Calendar c= StatUtils.getCalendarOfStatFile(f, new SimpleDateFormat(filePathFormat));

      reader = new BufferedReader(new InputStreamReader(fileSys.getInputStream(new File(baseDir, f))));

      //skip first line
      if (reader.readLine() == null) return true;

      String line;
      while ((line = reader.readLine()) != null) {
        try {
          CSVTokenizer tokenizer = new CSVTokenizer(line);

          //#1 MINSEC,USER,PAUSED,PLANNED,ACTIVE,FINISH,CANCEL,CREATED,DELETED

          if (tokenizer.hasMoreTokens()) {
            String minsec = tokenizer.nextToken();
            int minute = Integer.parseInt(minsec.substring(0, minsec.length() - 2));
            c.set(Calendar.MINUTE, minute);
            c.set(Calendar.SECOND, 0);

            Date date = c.getTime();
            if (filter.getFromDate() != null && date.before(filter.getFromDate()))
              continue;

            if (filter.getTillDate() != null && date.after(filter.getTillDate()))
              continue;

            String user = tokenizer.nextToken();
            if (filter != null && filter.getUser() != null && !filter.getUser().equals(user))
              continue;

            UserStatRecord rec = new UserStatRecord();
            rec.setUser(user);
            rec.setDate(Functions.convertTime(c.getTime(), STAT_TIMEZONE, LOCAL_TIMEZONE));

            rec.setPaused(Integer.parseInt(tokenizer.nextToken()));
            rec.setPlanned(Integer.parseInt(tokenizer.nextToken()));
            rec.setActive(Integer.parseInt(tokenizer.nextToken()));
            rec.setFinish(Integer.parseInt(tokenizer.nextToken()));
            rec.setCancel(Integer.parseInt(tokenizer.nextToken()));
            rec.setCreated(Integer.parseInt(tokenizer.nextToken()));
            rec.setDeleted(Integer.parseInt(tokenizer.nextToken()));

            if (!visitor.visit(rec, total, current))
              return false;
          }
        } catch (Exception e) {
          throw new DeliveryStatException("error.parsing.stat.line", f, line);
        }
      }
    } finally {
      if (reader != null) try {
        reader.close();
      }
      catch (IOException ignored) {
      }
    }
    return true;
  }

}
