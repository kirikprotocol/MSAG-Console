package mobi.eyeline.informer.admin.delivery.stat;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.CSVTokenizer;
import mobi.eyeline.informer.util.Functions;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.regex.Pattern;

/**
 * Интерфейс, обеспечивающий досуп к статистике
 *
 * @author Artem Snopkov
 */
public class DeliveryStatProvider {

  Logger log = Logger.getLogger(this.getClass());

  private static final TimeZone STAT_TIMEZONE=TimeZone.getTimeZone("UTC");

  private static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

  private File baseDir;
  private FileSystem fileSys;
  private String subDirNameFormat;
  private String filePathFormat;
  private Pattern fileNamePattern;

  public DeliveryStatProvider(File directory, FileSystem fileSys) {
    this(directory, fileSys, "yyyy.MM.dd");
  }

  protected DeliveryStatProvider(File directory, FileSystem fileSys, String subDirNamePattern) {
    baseDir = directory;
    this.fileSys = fileSys;
    this.subDirNameFormat = subDirNamePattern;
    this.fileNamePattern = Pattern.compile("msg\\d\\d\\.log");
    this.filePathFormat = subDirNameFormat + File.separatorChar + "'msg'HH'.log'";

  }

  /**
   * Поочередно передает в visitor все записи статистики, удовлетворяющие условиям, накладываемыми в filter.
   * Процесс продолжается до тех пор, пока метод visit в visitor возвращает true, либо записи не закончатся.
   * Если filter == null, то провайдер перебирает все записи.
   *
   * @param filter  фильтр, описывающий ограничения на записи
   * @param visitor визитор, обрабатывающий найденные записи
   * @throws AdminException если произошла ошибка при обращении к стораджу статистики
   */
  public void accept(DeliveryStatFilter filter, DeliveryStatVisitor visitor) throws AdminException {
    try {
      int minMinute = 0;
      int maxMinute = 59;


      if (filter != null && filter.getFromDate() != null) {
        Calendar c = Calendar.getInstance();
        c.setTime(filter.getFromDate());
        minMinute = c.get(Calendar.MINUTE);
      }
      if (filter != null && filter.getTillDate() != null) {
        Calendar c = Calendar.getInstance();
        c.setTime(filter.getTillDate());
        maxMinute = c.get(Calendar.MINUTE);
      }

      List<File> files = filterFiles(filter,true);
      Collections.sort(files);
      int total = files.size();
      for (int i = 0; i < total; i++) {
        File f = files.get(i);
        try {
          if (!process(f, filter, visitor, i, total, i == 0 ? minMinute : 0, i == total - 1 ? maxMinute : 59)) {
            break;
          }
        }
        catch (ParseException e) {
          log.error("Unparseable file path: " + f.getParent() + File.separatorChar + f.getName());
        }
      }
    }
    catch (IOException e) {
      throw new DeliveryException("filesys.ioexception");
    }
  }

  public List<File> filterFiles(DeliveryStatFilter filter, boolean endDateInclusive) throws AdminException {

    String minSubDirName = null;
    String maxSubDirName = null;
    String minFilePath = null;
    String maxFilePath = null;
    if (filter != null) {
      if (filter.getFromDate() != null) {
        Date fromDate = Functions.convertTime(filter.getFromDate(), LOCAL_TIMEZONE, STAT_TIMEZONE);
        minSubDirName = new SimpleDateFormat(subDirNameFormat).format(fromDate);
        minFilePath = new SimpleDateFormat(filePathFormat).format(fromDate);
      }
      if (filter.getTillDate() != null) {
        Date tillDate = Functions.convertTime(filter.getTillDate(), LOCAL_TIMEZONE, STAT_TIMEZONE);
        maxSubDirName = new SimpleDateFormat(subDirNameFormat).format(tillDate);
        maxFilePath = new SimpleDateFormat(filePathFormat).format(tillDate);
      }
    }


    List<File> files = new ArrayList<File>();
    if (fileSys.exists(baseDir)) {
      for (String subDirName : fileSys.list(baseDir)) {

        File subDir = new File(baseDir, subDirName);
        if (!subDir.isDirectory())
          continue;

        if (minSubDirName != null) {
          if (subDirName.compareTo(minSubDirName) < 0) {
            continue;
          }
        }
        if (maxSubDirName != null) {
          if (subDirName.compareTo(maxSubDirName) > 0) {
            continue;
          }
        }


        for (String fileName : fileSys.list(subDir)) {

          if (!fileNamePattern.matcher(fileName).matches())
            continue;

          String filePath = subDirName + File.separatorChar + fileName;
          if (minFilePath != null) {
            if (filePath.compareTo(minFilePath) < 0) {
              continue;
            }
          }
          if (maxFilePath != null && !endDateInclusive) {
            if (filePath.compareTo(maxFilePath) >= 0) {
              continue;
            }
          }
          if (maxFilePath != null && endDateInclusive) {
            if (filePath.compareTo(maxFilePath) > 0) {
              continue;
            }
          }

          files.add(new File(subDir, fileName));
        }
      }
    } else {
      log.error("Delivery statictics baseDir not exists:" + baseDir.getAbsolutePath());
    }
    return files;
  }


  boolean process(File file, DeliveryStatFilter filter, DeliveryStatVisitor visitor, int currentFile, int totalFilesCount, int fromMin, int toMin) throws AdminException, IOException, ParseException {
    InputStream is = null;
    try {

      Calendar c= getCalendarOfStatFile(file);

      is = fileSys.getInputStream(file);
      BufferedReader reader = new BufferedReader(new InputStreamReader(is));

      //skip first line
      if (reader.readLine() == null) return true;

      String line;
      while ((line = reader.readLine()) != null) {
        try {
          CSVTokenizer tokenizer = new CSVTokenizer(line);
          //# MINSEC,DLVID,USER,NEW,PROC,DLVD,FAIL,EXPD,DLVDSMS,FAILSMS,EXPDSMS
          if (tokenizer.hasMoreTokens()) {
            String minsec = tokenizer.nextToken();
            int minute = Integer.parseInt(minsec.substring(0, minsec.length() - 2));
            if (minute < fromMin) continue;
            if (minute > toMin) break;
            c.set(Calendar.MINUTE, minute);

            int taskId = Integer.parseInt(tokenizer.nextToken());
            if (filter != null && filter.getTaskIds() != null && !filter.getTaskIds().contains(taskId)) continue;

            String user = tokenizer.nextToken();
            if (filter != null && filter.getUser() != null && !filter.getUser().equals(user)) continue;

            int newmessages = Integer.parseInt(tokenizer.nextToken());
            int processing = Integer.parseInt(tokenizer.nextToken());
            int delivered = Integer.parseInt(tokenizer.nextToken());
            int failed = Integer.parseInt(tokenizer.nextToken());
            int expired = Integer.parseInt(tokenizer.nextToken());
            int deliveredSms = Integer.parseInt(tokenizer.nextToken());
            int failedSms = Integer.parseInt(tokenizer.nextToken());
            int expiredSms = Integer.parseInt(tokenizer.nextToken());

            DeliveryStatRecord rec = new DeliveryStatRecord(user, Functions.convertTime(c.getTime(), STAT_TIMEZONE, LOCAL_TIMEZONE), taskId,
                newmessages, processing, delivered, failed, expired,
                deliveredSms, failedSms, expiredSms
            );

            if (!visitor.visit(rec, totalFilesCount, currentFile)) {
              return false;
            }
          }
        }
        catch (Exception e) {
          throw new DeliveryStatException("error.parsing.stat.line", file.getAbsolutePath(), line);
        }
      }
    }
    finally {
      if (is != null) try {
        is.close();
      }
      catch (IOException ignored) {
      }
    }
    return true;
  }

  public Calendar getCalendarOfStatFile(File file) throws AdminException {
    try {
      String filePath = (new File(file.getParent())).getName() + File.separatorChar + file.getName();
      Date fileDate = new SimpleDateFormat(filePathFormat).parse(filePath);

      Calendar c = Calendar.getInstance();
      c.setTime(fileDate);
      return c;
    }
    catch (ParseException e) {
      throw new DeliveryStatException("error.parsing.filedate", file.getAbsolutePath());
    }
  }

}
