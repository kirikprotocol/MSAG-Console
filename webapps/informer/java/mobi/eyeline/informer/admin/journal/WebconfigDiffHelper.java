package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;

import java.util.Properties;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 12:33:14
 */
public class WebconfigDiffHelper extends DiffHelper {
  public WebconfigDiffHelper() {
    super(Subject.WEBCONFIG);
  }

  public void logUpdateJavaMailProps(Journal journal, Properties props, Properties old, String user) throws AdminException {
    logMapsDiff(journal, subject, "mail settings", old, props, user);
  }


  public void logUpdateNotificationTemplates(Journal journal, Properties props, Properties old, String user) throws AdminException {
    logMapsDiff(journal, subject, "notifivcation templates", old, props, user);
  }

  public void logUpdateSiebelProps(Journal journal, Properties props, Properties old, String user) throws AdminException {
    logMapsDiff(journal, subject, "siebel", old, props, user);
  }

  public void logUpdateSmsSenderAddress(Journal journal, Address addr, Address old, String user) throws AdminException {
    journal.addRecord(JournalRecord.Type.CHANGE, subject, user, "sms_sender_addr_changed", addr.getSimpleAddress(), old.getSimpleAddress());
  }
}
