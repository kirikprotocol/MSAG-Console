package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataSource;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyQuery;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;

import javax.servlet.http.HttpServletRequest;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;

/**
 * User: artem
 * Date: 01.11.2008
 */
public class RetryPolicyListBean extends InfoSmeBean {

  public static final int RESULT_EDIT = PRIVATE_RESULT + 1;
  public static final int RESULT_ADD = PRIVATE_RESULT + 2;

  private String mbAdd = null;
  private String mbEdit = null;
  private String mbDelete = null;

  private String sort = "name";
  private String edit = "";
  private String[] checked = new String[0];
  private Collection checkedSet = null;
  private QueryResultSet policies = new EmptyResultSet();
  private RetryPolicyDataSource ds;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    checkedSet = new HashSet(Arrays.asList(checked));
    ds = new RetryPolicyDataSource();
    return RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbEdit != null)
      return RESULT_EDIT;
    if (mbAdd != null)
      return RESULT_ADD;
    if (mbDelete != null)
      delete();

    try {
      policies = ds.query(getConfig(), new RetryPolicyQuery(1000, sort, 0));
    } catch (AdminException e) {
      e.printStackTrace();
      return error(e.getMessage());
    }

    return result;
  }

  private int delete() {
    try {
      for (int i = 0; i < getChecked().length; i++)
        ds.remove(getInfoSmeContext(), getChecked()[i]);
    } catch (Throwable e) {
      logger.error(e,e);
      return error(e.getMessage());
    }
    return RESULT_OK;
  }

  public QueryResultSet getPolicies() {
    return policies;
  }

  public boolean isPolicyChecked(String policy) {
    return checkedSet.contains(policy);
  }

  public String getEdit() {
    return edit;
  }

  public void setEdit(String edit) {
    this.edit = edit;
  }

  public String[] getChecked() {
    return checked;
  }

  public void setChecked(String[] checked) {
    this.checked = checked;
  }

  protected Collection getCheckedSet() {
    return checkedSet;
  }

  public String getSort() {
    return sort;
  }

  public void setSort(String sort) {
    this.sort = sort;
  }

  public String getMbAdd() {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd) {
    this.mbAdd = mbAdd;
  }

  public String getMbEdit() {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit) {
    this.mbEdit = mbEdit;
  }

  public String getMbDelete() {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete) {
    this.mbDelete = mbDelete;
  }
}
