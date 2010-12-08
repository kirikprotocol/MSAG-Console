package mobi.eyeline.informer.admin.delivery.stat;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
class StatFile  {

  private Date date;

  private String fileName;

  StatFile(Date date, String fileName) {
    this.date = date;
    this.fileName = fileName;
  }

  Date getDate() {
    return date;
  }

  String getFileName() {
    return fileName;
  }
}
