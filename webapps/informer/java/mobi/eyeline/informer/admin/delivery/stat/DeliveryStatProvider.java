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
 * Интерфейс, обеспечивающий досуп к статистике
 *
 * @author Artem Snopkov
 */
public class DeliveryStatProvider extends StatEntityProvider{

  private File baseDir;
  private FileSystem fileSys;
  private String filePathFormat;

  public DeliveryStatProvider(File directory, FileSystem fileSys) {
    this(directory, fileSys, "yyyy.MM.dd");
  }

  protected DeliveryStatProvider(File directory, FileSystem fileSys, String subDirNamePattern) {
    baseDir = directory;
    this.fileSys = fileSys;
    this.filePathFormat = subDirNamePattern + File.separatorChar + "'msg'HH'.log'";

  }

  public void visitEntities(Date from, Date till, StatEntityProvider.EntityVisitor visitor) throws AdminException {
    Date fromDate = prepareDateForFilesLookup(from);
    Date tillDate = prepareDateForFilesLookup(till);

    List<StatFile> files = StatUtils.lookupFiles(fileSys, baseDir, new SimpleDateFormat(filePathFormat), fromDate, tillDate);

    for (StatFile statFile : files) {
      if (!visitor.visit(new StatEntity(
          prepareDateForEntitiesView(statFile.getDate()),
          new File(baseDir, statFile.getFileName()).length()))
          ) {
        break;
      }
    }
  }

  @Override
  public synchronized void dropEntities(Date from, Date till) throws AdminException {
    Date fromDate = prepareDateForFilesLookup(from);
    Date tillDate = prepareDateForFilesLookup(till);

    List<StatFile> files = StatUtils.lookupFiles(fileSys, baseDir, new SimpleDateFormat(filePathFormat), fromDate, tillDate);

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
        if (fileSys.exists(_f)) {
          fileSys.delete(_f);
        }
        File _p = _f.getParentFile();
        if (_p != null) {
          parents.add(_f.getParentFile());
        }
      }
    }

    for(File _p : parents) {
      if(fileSys.exists(_p) && fileSys.list(_p).length == 0) {
        fileSys.delete(_p);
      }
    }
  }

  public void accept(DeliveryStatFilter filter, DeliveryStatVisitor visitor) throws AdminException {

    Date fromDate = filter == null ? null : prepareDateForFilesLookup(filter.getFromDate());
    Date tillDate = filter == null ? null : prepareDateForFilesLookup(filter.getTillDate());

    List<mobi.eyeline.informer.admin.delivery.stat.StatFile> files = StatUtils.lookupFiles(fileSys, baseDir, new SimpleDateFormat(filePathFormat), fromDate, tillDate);
    int total = files.size();

    DeliveryStatFilter convertedFilter = new DeliveryStatFilter();
    if(filter != null) {
      convertedFilter.setUser(filter.getUser());
      convertedFilter.setTaskIds(filter.getTaskIds());
      if (filter.getFromDate() != null)
        convertedFilter.setFromDate(Functions.convertTime(filter.getFromDate(), LOCAL_TIMEZONE, STAT_TIMEZONE));
      if (filter.getTillDate() != null)
        convertedFilter.setTillDate(Functions.convertTime(filter.getTillDate(), LOCAL_TIMEZONE, STAT_TIMEZONE));
    }

    try {
      for (int i=0; i<files.size(); i++)
        processFile(convertedFilter, visitor, total, i, files.get(i).getFileName());
    } catch (IOException e) {
      throw new DeliveryException("filesys.ioexception", e);
    }
  }

  public boolean processFile(DeliveryStatFilter filter, DeliveryStatVisitor visitor, int total, int current, String f) throws AdminException, IOException {

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

          //#1 MINSEC,USER,PAUSED,PLANNED,ACTIVE,FINISH,CANCEL,CREATED,DELETED,REGID
          //   MINSEC,DLVID,USER,NEW,PROC,DLVD,FAIL,EXPD,SMSDLVD,SMSFAIL,SMSEXPD,KILL,REGID
          if (tokenizer.hasMoreTokens()) {
            String minsec = tokenizer.nextToken();
            int minute = Integer.parseInt(minsec.substring(0, minsec.length() - 2));
            c.set(Calendar.MINUTE, minute);
            c.set(Calendar.SECOND, 59);

            Date date = c.getTime();
            if (filter.getFromDate() != null && date.before(filter.getFromDate()))
              continue;

            if (filter.getTillDate() != null && date.after(filter.getTillDate()))
              continue;

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
            Integer regionId = null;
            if(tokenizer.hasMoreTokens()) {
              tokenizer.nextToken(); //skip KILL
              if(tokenizer.hasMoreTokens()) {
                regionId = Integer.parseInt(tokenizer.nextToken());
              }
            }
            DeliveryStatRecord rec = new DeliveryStatRecord(user, Functions.convertTime(c.getTime(), STAT_TIMEZONE, LOCAL_TIMEZONE), taskId,
                newmessages, processing, delivered, failed, expired,
                deliveredSms, failedSms, expiredSms, regionId
            );

            if (!visitor.visit(rec, total, current)) {
              return false;
            }
          }
        } catch (Exception e) {
          logger.error(e,e);
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


//  boolean process(File file, DeliveryStatFilter filter, DeliveryStatVisitor visitor, int currentFile, int totalFilesCount, int fromMin, int toMin) throws AdminException, IOException, ParseException {
//    InputStream is = null;
//    try {
//
//      Calendar c= getCalendarOfStatFile(file);
//
//      is = fileSys.getInputStream(file);
//      BufferedReader reader = new BufferedReader(new InputStreamReader(is));
//
//      //skip first line
//      if (reader.readLine() == null) return true;
//
//      String line;
//      while ((line = reader.readLine()) != null) {
//        try {
//          CSVTokenizer tokenizer = new CSVTokenizer(line);
//          //# MINSEC,DLVID,USER,NEW,PROC,DLVD,FAIL,EXPD,DLVDSMS,FAILSMS,EXPDSMS
//          if (tokenizer.hasMoreTokens()) {
//            String minsec = tokenizer.nextToken();
//            int minute = Integer.parseInt(minsec.substring(0, minsec.length() - 2));
//            if (minute < fromMin) continue;
//            if (minute > toMin) break;
//            c.set(Calendar.MINUTE, minute);
//
//            int taskId = Integer.parseInt(tokenizer.nextToken());
//            if (filter != null && filter.getTaskIds() != null && !filter.getTaskIds().contains(taskId)) continue;
//
//            String user = tokenizer.nextToken();
//            if (filter != null && filter.getUser() != null && !filter.getUser().equals(user)) continue;
//
//            int newmessages = Integer.parseInt(tokenizer.nextToken());
//            int processing = Integer.parseInt(tokenizer.nextToken());
//            int delivered = Integer.parseInt(tokenizer.nextToken());
//            int failed = Integer.parseInt(tokenizer.nextToken());
//            int expired = Integer.parseInt(tokenizer.nextToken());
//            int deliveredSms = Integer.parseInt(tokenizer.nextToken());
//            int failedSms = Integer.parseInt(tokenizer.nextToken());
//            int expiredSms = Integer.parseInt(tokenizer.nextToken());
//
//            DeliveryStatRecord rec = new DeliveryStatRecord(user, Functions.convertTime(c.getTime(), STAT_TIMEZONE, LOCAL_TIMEZONE), taskId,
//                newmessages, processing, delivered, failed, expired,
//                deliveredSms, failedSms, expiredSms
//            );
//
//            if (!visitor.visit(rec, totalFilesCount, currentFile)) {
//              return false;
//            }
//          }
//        }
//        catch (Exception e) {
//          throw new DeliveryStatException("error.parsing.stat.line", file.getAbsolutePath(), line);
//        }
//      }
//    }
//    finally {
//      if (is != null) try {
//        is.close();
//      }
//      catch (IOException ignored) {
//      }
//    }
//    return true;
//  }

//  public Calendar getCalendarOfStatFile(File file) throws AdminException {
//    try {
//      String filePath = (new File(file.getParent())).getName() + File.separatorChar + file.getName();
//      Date fileDate = new SimpleDateFormat(filePathFormat).parse(filePath);
//
//      Calendar c = Calendar.getInstance();
//      c.setTime(fileDate);
//      return c;
//    }
//    catch (ParseException e) {
//      throw new DeliveryStatException("error.parsing.filedate", file.getAbsolutePath());
//    }
//  }


}
