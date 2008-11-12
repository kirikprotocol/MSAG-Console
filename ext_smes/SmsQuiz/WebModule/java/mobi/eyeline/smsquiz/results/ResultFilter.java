package mobi.eyeline.smsquiz.results;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ResultFilter implements Filter {

  private String address;

  private String quizId;


  public boolean isEmpty() {
    return false;
  }

  public boolean isItemAllowed(DataItem item) {
    if (!item.getValue("quizId").equals(quizId)) {
      return false;
    }
    return !((address != null) && (!item.getValue("msisdn").equals(address)));
  }

  public String getAddress() {
    return (address == null) ? "" : address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public String getQuizId() {
    return (quizId == null) ? "" : quizId;
  }

  public void setQuizId(String quizId) {
    this.quizId = quizId;
  }
}
