package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.delivery.MessageState;

import java.io.PrintStream;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * author: Aleksandr Khalitov
 */
class EyelineFileFormatStrategy implements FileFormatStrategy{

  private static ThreadLocal<DateFormat> dateFormat = new ThreadLocal<DateFormat>(){
    @Override protected DateFormat initialValue() {
      return new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");
    }
  };

  @Override
  public MessageInfo parseLine(String line) {
    int inx = line.indexOf('|');
    if (inx < 0)
      throw new IllegalArgumentException("INVALID LINE FORMAT");

    String abonent, text, userData = null, keywords = null;

    abonent = line.substring(0, inx).trim();
    int nextInx = line.indexOf('|', inx + 1);
    if (nextInx < 0)
      text = line.substring(inx + 1).trim();
    else {
      userData = line.substring(inx + 1, nextInx);
      inx = nextInx;
      nextInx = line.indexOf('|', inx+1);
      if(nextInx < 0) {
        text = line.substring(inx + 1).trim();
      }else {
        keywords = line.substring(inx + 1, nextInx).trim();
        text = line.substring(nextInx+1).trim();
      }
    }
    MessageInfo l = new MessageInfo();
    l.setMsisdn(abonent);
    l.setText(text);
    l.setUserData(userData);
    if(keywords != null && keywords.length() != 0) {
      l.setKeyword(keywords);
    }
    return l;
  }

  @Override
  public void writeReportLine(PrintStream reportWriter, String abonent, String userData, Date date, MessageState state, Integer smppCode) {
    reportWriter.print(abonent);
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
