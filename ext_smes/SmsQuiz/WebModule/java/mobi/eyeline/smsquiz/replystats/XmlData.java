package mobi.eyeline.smsquiz.replystats;

import java.util.Date;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class XmlData {

  private String address;
  private Date dateBegin;
  private Date dateEnd;

  public XmlData(String address, Date dateBegin, Date dateEnd) {
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
