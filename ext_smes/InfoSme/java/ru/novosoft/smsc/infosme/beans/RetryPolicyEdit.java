package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.config.retrypolicies.RetryPolicy;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.ErrorRetryPolicy;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataSource;
import ru.novosoft.smsc.jsp.util.helper.Validation;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.Column;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.IncorrectValueException;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.Row;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.RowControlButtonColumn;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.TextColumn;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * User: artem
 * Date: 01.11.2008
 */
public class RetryPolicyEdit extends InfoSmeBean {

  public static final int RESULT_SAVE = PRIVATE_RESULT + 1;
  public static final int RESULT_CANCEL = PRIVATE_RESULT + 2;

  private String mbSave = null;
  private String mbCancel = null;

  private boolean create = false;

  private String name;
  private int defaultPolicy;
  private List policies;

  private ErrorPoliciesTableHelper helper;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    helper = new ErrorPoliciesTableHelper("infosme.label.errorPolicies", "errorPolicies");
    return RESULT_OK;
  }
  
  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    try {
      helper.processRequest(request);
    } catch (IncorrectValueException e) {
      return error(e.getMessage());
    }

    if (mbSave != null && name != null) {
      mbSave = null;
      return save();
    } else if (mbCancel != null) {
      mbCancel = null;
      return RESULT_CANCEL;
    }

    if (name != null) {
      RetryPolicy p = getInfoSmeConfig().getRetryPolicy(name);
      defaultPolicy = p.getDefaultPolicy();
      policies = new ArrayList();
      for (Iterator iterator = p.getPolicies().entrySet().iterator(); iterator.hasNext();) {
        Map.Entry e = (Map.Entry)iterator.next();
        policies.add(new ErrorRetryPolicy((String)e.getKey(), ((Integer)e.getValue()).intValue()));
      }
      helper.fillTable();
    }

    return result;
  }

  private int save() {
    try {
      if (create && getInfoSmeConfig().containsRetryPolicy(name)) {
        return error("Retry policy with this name already exists");
      }
      RetryPolicy p = new RetryPolicy();
      p.setId(name);
      p.setDefaultPolicy(defaultPolicy);
      Map policies = new HashMap();
      for (Iterator iter = helper.getPolicies().iterator(); iter.hasNext();) {
        ErrorRetryPolicy erp = (ErrorRetryPolicy)iter.next();
        policies.put(erp.getErrorCode(), new Integer(erp.getPolicy()));
      }
      p.setPolicies(policies);
      getInfoSmeConfig().addRetryPolicy(p);
    } catch(Throwable e) {
      return error(e.getMessage());
    }
    return RESULT_SAVE;
  }

  public ErrorPoliciesTableHelper getHelper() {
    return helper;
  }

  public String getMbSave() {
    return mbSave;
  }

  public void setMbSave(String mbSave) {
    this.mbSave = mbSave;
  }

  public String getMbCancel() {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }

  public boolean isCreate() {
    return create;
  }

  public void setCreate(boolean create) {
    this.create = create;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public int getDefaultPolicy() {
    return defaultPolicy;
  }

  public void setDefaultPolicy(int defaultPolicy) {
    this.defaultPolicy = defaultPolicy;
  }

  public class ErrorPoliciesTableHelper extends DynamicTableHelper {

    private final Column errorColumn;
    private final Column policyColumn;
    private final Column delColumn;

    public ErrorPoliciesTableHelper(String name, String uid) {
      super(name, uid);
      errorColumn = new TextColumn(this, "infosme.label.error",  "error", 30, Validation.POSITIVE, true);
      policyColumn = new TextColumn(this, "infosme.label.policy", "policy", 30, Validation.POSITIVE, true);
      delColumn = new RowControlButtonColumn(this, "", "delColumn");
      addColumn(errorColumn);
      addColumn(policyColumn);
      addColumn(delColumn);
    }

    protected void fillTable() {
      for (Iterator iter = policies.iterator(); iter.hasNext();) {
        ErrorRetryPolicy policy = (ErrorRetryPolicy)iter.next();        
        Row row = createNewRow();
        row.addValue(errorColumn, policy.getErrorCode());
        row.addValue(policyColumn, String.valueOf(policy.getPolicy()));
      }
    }

    protected List getPolicies() {
      List result = new ArrayList(getRowsCount() + 1);
      for (Iterator iter = getRows(); iter.hasNext();) {
        Row row = (Row)iter.next();
        result.add(new ErrorRetryPolicy((String)row.getValue(errorColumn), Integer.parseInt((String)row.getValue(policyColumn))));
      }
      return result;
    }
  }
}
