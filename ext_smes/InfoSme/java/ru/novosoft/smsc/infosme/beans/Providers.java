package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.security.Principal;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:58:38 PM
 */
public class Providers extends InfoSmeBean
{
  public static final int RESULT_EDIT = InfoSmeBean.PRIVATE_RESULT + 0;
  public static final int RESULT_ADD = InfoSmeBean.PRIVATE_RESULT + 1;
  public static final int RESULT_REFRESH = InfoSmeBean.PRIVATE_RESULT + 2;
  protected static final int PRIVATE_RESULT = InfoSmeBean.PRIVATE_RESULT + 3;

  private String sort = null;
  private String mbEdit = null;
  private String mbAdd = null;
  private String mbDelete = null;
  private String edit = null;
  private String[] checkedProviders = new String[0];
  private Collection checkedProvidersSet = null;
  private boolean refreshed = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    checkedProvidersSet = new HashSet(Arrays.asList(checkedProviders));

    if (sort != null)
      getInfoSmeContext().setProvidersSort(sort);
    else
      sort = getInfoSmeContext().getProvidersSort();

    return result;
  }

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbEdit != null)
      return RESULT_EDIT;
    if (mbAdd != null)
      return RESULT_ADD;
    if (mbDelete != null)
      return delete();

    return refreshed ? result : RESULT_REFRESH;
  }

  private int delete()
  {
    for (int i = 0; i < checkedProviders.length; i++) {
      getConfig().removeSection("InfoSme.DataProvider." + StringEncoderDecoder.encodeDot(checkedProviders[i]));
      checkedProvidersSet.remove(checkedProviders[i]);
      getInfoSmeContext().setChangedProviders(true);
    }
    return RESULT_REFRESH;
  }

  public Collection getProviderNames()
  {
    if (sort.charAt(0) != '-')
      return new SortedList(getConfig().getSectionChildShortSectionNames("InfoSme.DataProvider"));
    else
      return new SortedList(getConfig().getSectionChildShortSectionNames("InfoSme.DataProvider"), new Comparator()
      {
        public int compare(Object o1, Object o2)
        {
          if (o1 instanceof String && o2 instanceof String)
            return -((String) o1).compareTo((String) o2);
          return 0;
        }
      });
  }

  public boolean isProviderChecked(String providerName)
  {
    return checkedProvidersSet.contains(providerName);
  }

  public String getSort()
  {
    return sort;
  }

  public void setSort(String sort)
  {
    this.sort = sort;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getEdit()
  {
    return edit;
  }

  public void setEdit(String edit)
  {
    this.edit = edit;
  }

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String[] getCheckedProviders()
  {
    return checkedProviders;
  }

  public void setCheckedProviders(String[] checkedProviders)
  {
    this.checkedProviders = checkedProviders;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public boolean isRefreshed()
  {
    return refreshed;
  }

  public void setRefreshed(boolean refreshed)
  {
    this.refreshed = refreshed;
  }
}
