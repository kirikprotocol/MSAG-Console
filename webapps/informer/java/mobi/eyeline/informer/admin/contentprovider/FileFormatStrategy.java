package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.delivery.MessageState;

import java.io.PrintStream;
import java.util.Date;

/**
* author: Aleksandr Khalitov
*/
interface FileFormatStrategy {

  MessageInfo parseLine(String line);

  void writeReportLine(PrintStream reportWriter, String abonent, String userData, Date date, MessageState state, Integer smppCode);

  static class MessageInfo {
    private String msisdn;
    private String text;
    private String userData;
    private String keyword;

    String getMsisdn() {
      return msisdn;
    }

    void setMsisdn(String msisdn) {
      this.msisdn = msisdn;
    }

    String getText() {
      return text;
    }

    void setText(String text) {
      this.text = text;
    }

    String getUserData() {
      return userData;
    }

    void setUserData(String userData) {
      this.userData = userData;
    }

    public String getKeyword() {
      return keyword;
    }

    void setKeyword(String keyword) {
      this.keyword = keyword;
    }
  }

}
