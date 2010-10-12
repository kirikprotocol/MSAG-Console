package mobi.eyeline.informer.web.controllers.retry_policies;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicy;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicyEntry;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableRow;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.10.2010
 * Time: 15:26:51
 */
public class RetryPolicyEditController extends InformerController {

  String policyId;
  RetryPolicy policy;

  private static final String POLICY_ID_PARAMETER = "policyId";
  private boolean initError;
  private DynamicTableModel dynamicModel = new DynamicTableModel();


  public RetryPolicyEditController() {
    super();
    try {
      setPolicyId(getRequestParameter(POLICY_ID_PARAMETER));
    }
    catch (AdminException e){
      initError = true;
    }
  }

  void initModel() {
    dynamicModel = new DynamicTableModel();
    for(RetryPolicyEntry entry : policy.getEntries()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("errorCode",entry.getErrCode());
      row.setValue("time",entry.getTime());
      dynamicModel.addRow(row);
    }
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }

  public String getPolicyId() {
    return policyId;
  }

  private void setPolicyId(String policyId) throws AdminException {
    if(policyId==null || policyId.length()==0) {
      policyId=null;
      policy  = new RetryPolicy();
    }
    else {
      policy  = getConfig().getRetryPolicy(policyId);
    }
    initModel();
    this.policyId = policyId;
  }


  public String save() {
    Configuration config = getConfig();
    try {
      List<RetryPolicyEntry> entries = new ArrayList<RetryPolicyEntry>();
      for(DynamicTableRow row : dynamicModel.getRows()) {
        Integer errCode;
        Integer time;
        try {
          errCode = Integer.valueOf((String) row.getValue("errorCode"));
        }
        catch (NumberFormatException ex){
          addLocalizedMessage(FacesMessage.SEVERITY_WARN,"retry_policy.err.errCode.isNAN",row.getValue("errorCode"));
          return null;
        }
        try {
          time = Integer.valueOf((String) row.getValue("time"));
        }
        catch (NumberFormatException ex){
          addLocalizedMessage(FacesMessage.SEVERITY_WARN,"retry_policy.err.time.isNAN",row.getValue("errorCode"));
          return null;
        }
        entries.add(new RetryPolicyEntry(errCode,time));
        policy.setEntries(entries);
      }
    }
    catch (AdminException e) {
      addError(e);
      return null;
    }

    try{
      if(policyId==null) {
        config.addRetryPolicy(policy,getUserName());
      }
      else {
        if(policyId.equals(policy.getPolicyId())) {
          config.updateRetryPolicy( policy,getUserName());
        }
        else {
          config.addRetryPolicy( policy,getUserName());
          config.removeRetryPolicy(policyId,getUserName());
        }
      }
    }
    catch (AdminException e){
      addError(e);
      return null;
    }
    return "RETRY_POLICIES";  //To change body of created methods use File | Settings | File Templates.
  }

  public RetryPolicy getPolicy() {
    return policy;
  }

  public void setDynamicModel(DynamicTableModel dynamicModel) {
    this.dynamicModel = dynamicModel;
  }

  public DynamicTableModel getDynamicModel() {
    return dynamicModel;
  }
}
