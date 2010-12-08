package mobi.eyeline.informer.admin.delivery.stat;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class StatEntity {

  private Date date;

  private long size;  

  StatEntity(Date date, long size) {
    this.date = date;
    this.size = size;
  }

  public Date getDate() {
    return date;
  }

  public long getSize() {
    return size;
  }
}
