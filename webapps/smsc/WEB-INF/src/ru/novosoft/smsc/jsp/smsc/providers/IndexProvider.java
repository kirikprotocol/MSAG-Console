package ru.novosoft.smsc.jsp.smsc.providers;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.provider.ProviderQuery;

import javax.servlet.http.HttpServletRequest;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class IndexProvider extends IndexBean
{
  public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
  public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
  public static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;
  private long id = -1;
  private String name = null;
  protected String editName = null;
  protected String editId = null;
  //private boolean add = false;
  protected QueryResultSet providers = null;
  protected ProviderManager providerManager = null;
  protected String mbAdd = null;
  protected String mbDelete = null;
  protected String mbEdit = null;
  protected String[] checkedProviderNames = new String[0];
  protected Set checkedProviderNamesSet = new HashSet();

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;
    pageSize = preferences.getProvidersPageSize();
    if (sort != null)
      preferences.setProvidersSortOrder(sort);
    else
      sort = preferences.getProvidersSortOrder();

    providerManager = appContext.getProviderManager();
    return RESULT_OK;
  }

  public int process(final HttpServletRequest request)
  {
    super.process(request);
    providers = new EmptyResultSet();

    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbAdd != null)
      return RESULT_ADD;
    else if (mbEdit != null)
      return RESULT_EDIT;
    else if (mbDelete != null) {
      int dresult = deleteProviders();
      if (dresult != RESULT_OK)
        return result;
      else
        return RESULT_DONE;
    }

    logger.debug("Providers.Index - process with sorting [" + preferences.getProvidersSortOrder() + "]");
    ProviderQuery query = new ProviderQuery(pageSize, preferences.getProviderFilter(), preferences.getProvidersSortOrder(), startPosition);
    providers = appContext.getProviderManager().query(query);
    totalSize = providers.getTotalSize();

    checkedProviderNamesSet.addAll(Arrays.asList(checkedProviderNames));

    return RESULT_OK;
  }

  private int deleteProviders()
  {
    for (int i = 0; i < checkedProviderNames.length; i++) {
      String checkedProviderName = checkedProviderNames[i];
      providerManager.removeProvider(checkedProviderName);
      journalAppend(SubjectTypes.TYPE_provider, checkedProviderName, Actions.ACTION_DEL);
    }
    setCheckedProviderNames(new String[0]);
    appContext.getStatuses().setProvidersChanged(true);
    return RESULT_OK;
  }

  /**
   * ***************************** properties *************************************
   */

  public boolean isProviderChecked(String name)
  {
    return checkedProviderNamesSet.contains(name);
  }

  public String[] getCheckedProviderNames()
  {
    return checkedProviderNames;
  }

  public void setCheckedProviderNames(String[] checkedProviderNames)
  {
    this.checkedProviderNames = checkedProviderNames;
  }

  public String getEditName()
  {
    return editName;
  }

  public void setEditName(String editName)
  {
    this.editName = editName;
  }

  public String getEditId()
  {
    return editId;
  }

  public void setEditId(String editId)
  {
    this.editId = editId;
  }

  public String getId()
  {
    return -1 == id ? null : String.valueOf(id);
  }

  public void setId(final String id)
  {
    this.id = Long.decode(id).longValue();
  }

  public String getName()
  {
    return name;
  }

  public void setName(final String name)
  {
    this.name = name;
  }


  public QueryResultSet getProviders()
  {
    return providers;
  }


  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public ProviderManager getProviderManager()
  {
    return providerManager;
  }

  public void setProviderManager(ProviderManager providerManager)
  {
    this.providerManager = providerManager;
  }

  public Set getCheckedProviderNamesSet()
  {
    return checkedProviderNamesSet;
  }

  public void setCheckedProviderNamesSet(Set checkedProviderNamesSet)
  {
    this.checkedProviderNamesSet = checkedProviderNamesSet;
  }

  public void setProviders(QueryResultSet providers)
  {
    this.providers = providers;
  }
/*  protected void load(final String loadId) throws SMSCJspException
  {
    final Map providers = appContext.getProviderManager().getProviders();
    final Long longLoadId = Long.decode(loadId);
    if (!providers.containsKey(longLoadId))
    //  throw new SMSCJspException(Constants.errors.providers.PROVIDER_NOT_FOUND, loadId);
        error(SMSCErrors.error.providers.provider_not_found, String.valueOf(longLoadId));
    final Provider info = (Provider) providers.get(longLoadId);
    this.id = info.getId();
    this.name = info.getName();
  }
  */

/*  protected void save() throws SMSCJspException
  {
    final ProviderManager providerManager = appContext.getProviderManager();
    if (isAdd()) {
      providerManager.createProvider(name);
    } else {
      providerManager.setProviderName(id, name);
    }
    appContext.getStatuses().setProvidersChanged(true);
   // throw new DoneException();
  }
  */
}
