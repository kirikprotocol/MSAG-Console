package mobi.eyeline.smsquiz;

import java.util.Date;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class QuizXmlData {

  private String address;
  private Date dateBegin;
  private Date dateEnd;

  public QuizXmlData(String address, Date dateBegin, Date dateEnd) {
    this.address = address;
    this.dateBegin = dateBegin;
    this.dateEnd = dateEnd;
  }

  public String getAddress() {
    return address;
  }


  public Date getDateBegin() {
    return dateBegin;
  }

  public Date getDateEnd() {
    return dateEnd;
  }
}
