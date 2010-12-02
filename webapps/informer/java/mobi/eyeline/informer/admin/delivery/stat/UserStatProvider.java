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
public class UserStatProvider {

  private static final TimeZone STAT_TIMEZONE=TimeZone.getTimeZone("UTC");
  private static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

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

  private static Date prepareDateForFilesLookup(Date date) {
    if (date == null)
      return null;
    date = Functions.convertTime(date, LOCAL_TIMEZONE, STAT_TIMEZONE);
    Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    return c.getTime();
  }

  public List<File> filterFiles(UserStatFilter filter, boolean endDateInclusive) throws AdminException {
    Date fromDate = prepareDateForFilesLookup(filter.getFromDate());
    Date tillDate = prepareDateForFilesLookup(filter.getTillDate());

    List<String> files = StatUtils.lookupFiles(baseDir, new SimpleDateFormat(filePathFormat), fromDate, tillDate);
    int last = files.size();
    if (!endDateInclusive)
      last--;

    List<File> result = new ArrayList<File>();
    for (int i=0; i<last; i++)
      result.add(new File(baseDir, files.get(i)));
    return result;
  }

  public void accept(UserStatFilter filter, UserStatVisitor visitor) throws AdminException {

    Date fromDate = prepareDateForFilesLookup(filter.getFromDate());
    Date tillDate = prepareDateForFilesLookup(filter.getTillDate());

    List<String> files = StatUtils.lookupFiles(baseDir, new SimpleDateFormat(filePathFormat), fromDate, tillDate);
    int total = files.size();

    UserStatFilter convertedFilter = new UserStatFilter();
    convertedFilter.setUser(filter.getUser());
    if (filter.getFromDate() != null)
      convertedFilter.setFromDate(Functions.convertTime(filter.getFromDate(), LOCAL_TIMEZONE, STAT_TIMEZONE));
    if (filter.getTillDate() != null)
      convertedFilter.setTillDate(Functions.convertTime(filter.getTillDate(), LOCAL_TIMEZONE, STAT_TIMEZONE));

    try {
      for (int i=0; i<files.size(); i++)
        processFile(convertedFilter, visitor, total, i, files.get(i));
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
