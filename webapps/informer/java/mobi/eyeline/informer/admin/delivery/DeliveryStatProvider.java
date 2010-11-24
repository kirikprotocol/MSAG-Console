package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.CSVTokenizer;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Интерфейс, обеспечивающий досуп к статистике
 *
 * @author Artem Snopkov
 */
class DeliveryStatProvider {

  Logger log = Logger.getLogger(this.getClass());

  private File baseDir;
  private FileSystem fileSys;
  private String subDirNameFormat;
  private String filePathFormat;

  DeliveryStatProvider(File directory, FileSystem fileSys) {
    this(directory, fileSys, "yyyy.MM.dd");
  }

  DeliveryStatProvider(File directory, FileSystem fileSys, String subDirNamePattern) {
    baseDir = directory;
    this.fileSys = fileSys;
    this.subDirNameFormat = subDirNamePattern;
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
  void accept(DeliveryStatFilter filter, DeliveryStatVisitor visitor) throws AdminException {
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

      List<File> files = filterFiles(filter);
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

  private List<File> filterFiles(DeliveryStatFilter filter) throws AdminException {

    String minSubDirName = null;
    String maxSubDirName = null;
    String minFilePath = null;
    String maxFilePath = null;
    if (filter != null) {
      if (filter.getFromDate() != null) {
        minSubDirName = new SimpleDateFormat(subDirNameFormat).format(filter.getFromDate());
        minFilePath = new SimpleDateFormat(filePathFormat).format(filter.getFromDate());
      }
      if (filter.getTillDate() != null) {
        maxSubDirName = new SimpleDateFormat(subDirNameFormat).format(filter.getTillDate());
        maxFilePath = new SimpleDateFormat(filePathFormat).format(filter.getTillDate());
      }
    }


    List<File> files = new ArrayList<File>();
    if (fileSys.exists(baseDir)) {
      for (String subDirName : fileSys.list(baseDir)) {
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
        File subDir = new File(baseDir, subDirName);
        for (String fileName : fileSys.list(subDir)) {
          String filePath = subDirName + File.separatorChar + fileName;
          if (minFilePath != null) {
            if (filePath.compareTo(minFilePath) < 0) {
              continue;
            }
          }
          if (maxFilePath != null) {
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

      String filePath = (new File(file.getParent())).getName() + File.separatorChar + file.getName();
      Date fileDate = new SimpleDateFormat(filePathFormat).parse(filePath);
      Calendar c = Calendar.getInstance();
      c.setTime(fileDate);

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
            if (filter != null && filter.getTaskId() != null && filter.getTaskId() != taskId) continue;

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

            DeliveryStatRecord rec = new DeliveryStatRecord(user, c.getTime(), taskId,
                newmessages, processing, delivered, failed, expired,
                deliveredSms, failedSms, expiredSms
            );

            if (!visitor.visit(rec, totalFilesCount, currentFile)) {
              return false;
            }
          }
        }
        catch (Exception e) {
          throw new DeliveryStatException("error.parsing.stat.line", filePath, line);
        }
      }
    }
    finally {
      if (is != null) try {
        is.close();
      }
      catch (IOException e) {
      }
    }
    return true;
  }

}
