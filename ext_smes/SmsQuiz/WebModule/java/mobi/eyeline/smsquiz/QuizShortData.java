package mobi.eyeline.smsquiz;

import java.util.Date;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class QuizShortData {

  private String address;
  private Date dateBegin;
  private Date dateEnd;
  private String abFile;

  public QuizShortData(String address, Date dateBegin, Date dateEnd, String abFile) {
    this.address = address;
    this.dateBegin = dateBegin;
    this.dateEnd = dateEnd;
    this.abFile = abFile;
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

  public String getAbFile() {
    return abFile;
  }
}
