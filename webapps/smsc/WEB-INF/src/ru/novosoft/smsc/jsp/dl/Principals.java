package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.dl.PrincipalsDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.dl.PrincipalsFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.dl.PrincipalsQuery;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.DistributionListManager;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;

import javax.servlet.http.HttpServletRequest;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 22.09.2003
 * Time: 15:50:25
 * To change this template use Options | File Templates.
 */
public class Principals extends IndexBean
{
  public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT;
  public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT + 1;
  protected static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

  private String edit = null;
  private QueryResultSet principals = new EmptyResultSet();
  private String[] checked = new String[0];
  private Set checkedSet = new HashSet();
  private String filterAddress = null;

  private String mbAdd = null;
  private String mbEdit = null;
  private String mbDelete = null;
  private PrincipalsFilter filter;
  private boolean initialized = false;
  DistributionListAdmin admin=null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;
    if (this.admin == null)
      admin = appContext.getSmsc().getDistributionListAdmin();

    filter = preferences.getDlPrincipalsFilter();
    if (filterAddress == null) {
      if (initialized)
        filter.setFilterAddress(filterAddress = "*");
      else
        filterAddress = filter.getFilterAddress();
    }
    else
      filter.setFilterAddress(filterAddress);
    if (filter.isEmpty())
      filterAddress = "*";

    if (sort == null)
      sort = preferences.getDlPrincipalsSortOrder();
    else
      preferences.setDlPrincipalsSortOrder(sort);

    if (pageSize < 1)
      pageSize = preferences.getDlPrincipalsPageSize();
    else
      preferences.setDlPrincipalsPageSize(pageSize);

    checkedSet.addAll(Arrays.asList(checked));
    if (initialized)
      principals = new PrincipalsDataSource(appContext).query(new PrincipalsQuery(pageSize, filter, sort, startPosition),admin);
    totalSize = principals.getTotalSize();

    return result;
  }
  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbAdd != null) return RESULT_ADD;
    if (mbEdit != null) return RESULT_EDIT;
    if (mbDelete != null) return delete();
    return result;
  }

  private int delete()
  {
    int result = RESULT_DONE;
    //  PreparedStatement statement = connection.prepareStatement("delete from dl_principals where address=?");

    for (int i = 0; i < checked.length; i++) {
      String address = checked[i];
      try {
        admin.deletePrincipal(address);
      } catch (AdminException e) {
        e.printStackTrace();
        logger.error("Could not delete principal \"" + address + "\"", e);
        result = error(SMSCErrors.error.dl.couldntDeletePrincipal, address, e);
      }
    }
    return result;
  }

  public QueryResultSet getPrincipals()
  {
    return principals;
  }

  public boolean isChecked(String address)
  {
    return checkedSet.contains(address);
  }

  public String getEdit()
  {
    return edit;
  }

  public void setEdit(String edit)
  {
    this.edit = edit;
  }

  public String[] getChecked()
  {
    return checked;
  }

  public void setChecked(String[] checked)
  {
    this.checked = checked;
  }

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public String getFilterAddress()
  {
    return filterAddress;
  }

  public void setFilterAddress(String filterAddress)
  {
    this.filterAddress = filterAddress;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }
}
