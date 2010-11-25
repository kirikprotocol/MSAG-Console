package mobi.eyeline.informer.admin.contentprovider;

import java.io.PrintStream;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 25.11.2010
 * Time: 12:46:26
 */
class ContentProviderReportFormatter {

  static ThreadLocal<DateFormat> dateFormat = new ThreadLocal<DateFormat>(){
    @Override protected DateFormat initialValue() {
      return new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");
    }
  };

  static void writeReportLine(PrintStream reportWriter, String abonent, Date date, String s) {
    reportWriter.print(abonent);
    reportWriter.print(" | ");
    reportWriter.print(dateFormat.get().format(date));
    reportWriter.print(" | ");
    reportWriter.println(s);
  }
}
