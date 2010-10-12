package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicy;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicyEntry;
import mobi.eyeline.informer.admin.smsc.Smsc;

import java.lang.reflect.Method;
import java.util.List;
import java.util.Map;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.10.2010
 * Time: 12:01:13
 */
public class RetryPolicyDiffHelper extends DiffHelper {

  public RetryPolicyDiffHelper(Subject subj) {
    super(subj);
  }

  public void logUpdateRetryPolicy(Journal journal, RetryPolicy oldRp, RetryPolicy newRp, String userName) throws AdminException {
    if(oldRp.getDefaultTime()!=newRp.getDefaultTime()) {
      //Retry Policy '{3}' property changed:  name={0}, old val='{1}', new val='{2}'
      journal.addRecord(JournalRecord.Type.CHANGE, subject, userName, "retry_policy_changed", "defaultTime",oldRp.getDefaultTime()+"",newRp.getDefaultTime()+"",newRp.getPolicyId());
    }
    logMapsDiff(journal,subject,oldRp.getPolicyId(),oldRp.getEntryMap(),newRp.getEntryMap(),userName);
  }

  public void logAddRetryPolicy(Journal journal, RetryPolicy rp, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.ADD, subject, userName, "retry_policy_added", rp.toString());
  }

  public void logRemoveRetryPolicy(Journal journal, String policyId, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.REMOVE, subject, userName, "retry_policy_removed", policyId);
  }

}
