package ru.novosoft.smsc.infosme.backend.tables.retrypolicies;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.infosme.backend.InfoSme;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.infosme.backend.config.retrypolicies.RetryPolicy;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskQuery;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.AdminException;

import java.util.*;
import java.io.IOException;

/**
 * User: artem
 * Date: 01.11.2008
 */
public class RetryPolicyDataSource extends AbstractDataSourceImpl {

  public static final String RETRY_POLICIES_SECTION = "InfoSme.RetryPolicies";

  public RetryPolicyDataSource() {
    super(new String[]{"name", "errors"});
  }

//  public void save(InfoSmeContext ctx, RetryPolicyDataItem policy) {
//    Config infoSmeConfig = ctx.getConfig();
//    infoSmeConfig.removeSection(RETRY_POLICIES_SECTION + '.' + policy.getName());
//    String secName = RETRY_POLICIES_SECTION + '.' + policy.getName() + '.';
//    infoSmeConfig.setInt(secName + "default", policy.getDefaultPolicy());
//    for (Iterator iter = policy.getErrorsPolicies().iterator(); iter.hasNext();) {
//      ErrorRetryPolicy e = (ErrorRetryPolicy)iter.next();
//      infoSmeConfig.setInt(secName + e.getErrorCode(), e.getPolicy());
//    }
//    ctx.setChangedRetryPolicies(true);
//  }
//
//  public boolean remove(InfoSmeContext ctx, String retryPolicyName) throws AdminException {
//    try {
//      // Check retry policy is used in any task
//      final TaskDataSource tds = new TaskDataSource(ctx.getInfoSme(), ctx.getConfig());
//      QueryResultSet rs = tds.query(new TaskQuery(1000, "name", 0));
//      for (int i=0; i<rs.getTotalSize(); i++) {
//        TaskDataItem task = (TaskDataItem)rs.get(i);
//        if (task.getRetryPolicy() != null && task.getRetryPolicy().equals(retryPolicyName))
//          throw new AdminException("Retry policy " + retryPolicyName + " is used in task " + task.getName());
//      }
//
//      // Remove retry policy
//      ctx.getConfig().removeSection(RETRY_POLICIES_SECTION + '.' + retryPolicyName);
//      ctx.setChangedRetryPolicies(true);
//      return true;
//    } catch (Exception e) {
//      e.printStackTrace();
//      throw new AdminException(e.getMessage());
//    }
//  }
//
//  public RetryPolicyDataItem get(Config infoSmeConfig, String retryPolicyName) throws AdminException {
//    String sec = RETRY_POLICIES_SECTION + '.' + retryPolicyName;
//    if (!infoSmeConfig.containsSection(sec))
//      return null;
//    try {
//      int defaultPolicy = infoSmeConfig.getInt(sec + ".default");
//      return new RetryPolicyDataItem(retryPolicyName, defaultPolicy, getErrorPolicies(infoSmeConfig, sec));
//    } catch (Throwable e) {
//      throw new AdminException(e.getMessage());
//    }
//  }

  public QueryResultSet query(InfoSmeConfig infoSmeConfig, RetryPolicyQuery query_to_run) throws AdminException {
    for (Iterator iter = infoSmeConfig.getRetryPolicies().iterator(); iter.hasNext();) {
      RetryPolicy p = (RetryPolicy)iter.next();
      List errorPolicies = new ArrayList();
      for (Iterator policies = p.getPolicies().entrySet().iterator(); policies.hasNext();) {
        Map.Entry e = (Map.Entry)policies.next();
        errorPolicies.add(new ErrorRetryPolicy((String)e.getKey(), ((Integer)e.getValue()).intValue()));
      }
      add(new RetryPolicyDataItem(p.getId(), p.getDefaultPolicy(), errorPolicies));
    }
    return super.query(query_to_run);
  }

//  private static List getErrorPolicies(Config infoSmeConfig, String policyName) throws Config.WrongParamTypeException, Config.ParamNotFoundException {
//    List result = new LinkedList();
//    Collection errorPolicies = infoSmeConfig.getSectionChildShortParamsNames(policyName);
//    for (Iterator iter1 = errorPolicies.iterator(); iter1.hasNext();) {
//      String errors = (String)iter1.next();
//      if (errors.equals("default"))
//        continue;
//      int policy = infoSmeConfig.getInt(policyName + '.' + errors);
//      StringTokenizer st = new StringTokenizer(errors, ",");
//      while (st.hasMoreTokens()) {
//        String token = st.nextToken();
//        result.add(new ErrorRetryPolicy(token, policy));
//      }
//    }
//    return result;
//  }
}
