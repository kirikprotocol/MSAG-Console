package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.cdr.CdrSettings;
import mobi.eyeline.informer.admin.notifications.NotificationSettings;
import mobi.eyeline.informer.admin.siebel.SiebelSettings;
import mobi.eyeline.informer.util.Address;

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

  public void logUpdateNotificationSettings(Journal journal, NotificationSettings props, NotificationSettings old, String user) throws AdminException {
    Address p = props.getSmsSenderAddress();
    Address o = old.getSmsSenderAddress();

    if((p == null && o != null) || (p != null && o == null) || (p != null && !p.equals(o))) {
      journal.addRecord(JournalRecord.Type.CHANGE, subject, user, "sms_sender_addr_changed",
         o == null ? null : o.toString(),  p == null ? null : p.toString());
    }

    logMapsDiff(journal, subject, "mail settings", old.getMailProperties(), props.getMailProperties(), user);

    logMapsDiff(journal, subject, "notification settings", old.getNotificationTemplates(), props.getNotificationTemplates(), user);
  }



  public void logUpdateSiebelProps(Journal journal, SiebelSettings props, SiebelSettings old, String user) throws AdminException {
    logMapsDiff(journal, subject, "siebel", old.getAllProperties(), props.getAllProperties(), user);
  }
  
  public void logUpdateCdrProps(Journal journal, CdrSettings props, CdrSettings old, String user) throws AdminException {
    logMapsDiff(journal, subject, "cdr", old.getAllProperties(), props.getAllProperties(), user);
  }

}
