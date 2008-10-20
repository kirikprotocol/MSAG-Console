package mobi.eyeline.smsquiz.distribution;

import mobi.eyeline.smsquiz.storage.StorageException;

import java.util.Date;

import org.apache.log4j.Logger;

/**
 * author: alkhal
 */
public class StatsDelivery {

  private static final Logger logger = Logger.getLogger(StatsDelivery.class);

  private String msisdn;
  private Date date;

  public StatsDelivery(String msisdn, Date date) throws StorageException {
    if ((msisdn == null) || (date == null)) {
      logger.error("Some arguments are null");
      throw new StorageException("Some arguments are null", StorageException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    this.msisdn = msisdn;
    this.date = date;
  }

  public String getMsisdn() {
    return msisdn;
  }

  public Date getDate() {
    return date;
  }

  public String toString() {
    return msisdn + " " + date;
  }

}
