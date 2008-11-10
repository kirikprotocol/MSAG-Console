package mobi.eyeline.smsquiz.quizes.view;

import java.util.Date;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class QuizShortData {

  private String address;
  private Date dateBegin;
  private Date dateEnd;

  public QuizShortData(String address, Date dateBegin, Date dateEnd) {
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
