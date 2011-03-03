package mobi.eyeline.informer.admin.delivery.stat;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Artem Snopkov
 */
class StatUtils {

  static List<StatFile> lookupFiles(FileSystem fs, File baseDir, SimpleDateFormat filePathFormat, Date fromDate, Date tillDate) {
    if (baseDir == null || !fs.isDirectory(baseDir))
      return Collections.emptyList();

    List<StatFile> result = new ArrayList<StatFile>();

    recursivellyLookupFiles(fs, baseDir, null, filePathFormat, fromDate, tillDate, result);

    return result;
  }

  private static void recursivellyLookupFiles(FileSystem fs, File baseDir, String prefix, SimpleDateFormat filePathFormat, Date fromDate, Date tillDate, List<StatFile> result) {

    File[] files = fs.listFiles(baseDir);
    if (files == null)
      return;

    for (File f : files) {
      String fileName = prefix;
      if (fileName == null)
        fileName = f.getName();
      else
        fileName += File.separator + f.getName();

      if (fs.isDirectory(f))
        recursivellyLookupFiles(fs, f, fileName, filePathFormat, fromDate, tillDate, result);
      if (!fs.isDirectory(f)) {
        Date fileDate;
        try {
          fileDate = filePathFormat.parse(fileName);
        } catch (ParseException e) {
          continue;
        }
        if (fromDate != null && fileDate.before(fromDate))
          continue;
        if (tillDate != null && fileDate.after(tillDate))
          continue;
        result.add(new StatFile(fileDate, fileName));
      }
    }
  }

  static Calendar getCalendarOfStatFile(String filePath, SimpleDateFormat filePathFormat) throws AdminException {
    try {
      Date fileDate = filePathFormat.parse(filePath);

      Calendar c = Calendar.getInstance();
      c.setTime(fileDate);
      return c;
    }
    catch (ParseException e) {
      throw new DeliveryStatException("error.parsing.filedate", filePath);
    }
  }

}
