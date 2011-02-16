package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.util.Address;

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
class ReportFormatter {

  private static ThreadLocal<DateFormat> dateFormat = new ThreadLocal<DateFormat>(){
    @Override protected DateFormat initialValue() {
      return new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");
    }
  };

  static void writeReportLine(PrintStream reportWriter, Address abonent, String userData, Date date, MessageState state, Integer smppCode) {
    reportWriter.print(abonent.getSimpleAddress());
    reportWriter.print("|");
    if (userData != null) {
      reportWriter.print(userData);
      reportWriter.print("|");
    }
    reportWriter.print(dateFormat.get().format(date));
    reportWriter.print("|");
    reportWriter.print(state.toString());
    reportWriter.print("|");
    if (smppCode == null)
      smppCode = 0;
    reportWriter.println(smppCode);
  }
}
