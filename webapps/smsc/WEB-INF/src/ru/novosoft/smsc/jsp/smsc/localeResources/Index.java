package ru.novosoft.smsc.jsp.smsc.localeResources;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.util.SortedList;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: Feb 18, 2003
 * Time: 10:51:44 AM
 */
public class Index extends IndexBean
{
/*
	public static final String PREFIX = "resources_";
	public static final String BODY_PATTERN = ".{2,2}_.{2,2}";
	public static final String SUFFIX = "\\.xml";
	public static final String PATTERN = PREFIX + BODY_PATTERN + SUFFIX;
	public static final int PREFIX_LENGTH = PREFIX.length();
	public static final int BODY_LENGTH = 5;
*/

  private List locales = null;
  private List reverseLocales = new LinkedList();
  private String[] checkedLocales = new String[0];
  private Set checkedLocalesSet = new HashSet();
  private String viewLocaleResource = null;

  private String mbAdd = null;
  private String mbDelete = null;
  private String mbView = null;

  public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
  public static final int RESULT_VIEW = IndexBean.PRIVATE_RESULT + 1;
  protected static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    pageSize = preferences.getLocaleResourcesPageSize();
    if (sort != null)
      preferences.setLocaleResourcesSortOrder(sort);
    else
      sort = preferences.getLocaleResourcesSortOrder();

    locales = new SortedList(appContext.getResourcesManager().list());
    reverseLocales.addAll(locales);
    Collections.reverse(reverseLocales);

    if (checkedLocales == null)
      checkedLocales = new String[0];

    totalSize = locales.size();

    checkedLocalesSet.clear();
    checkedLocalesSet.addAll(Arrays.asList(checkedLocales));
    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbDelete != null)
      return delete();
    else if (mbView != null)
      return RESULT_VIEW;
    else if (mbAdd != null)
      return RESULT_ADD;

    return RESULT_OK;
  }

  private int delete()
  {
    locales.removeAll(checkedLocalesSet);
    reverseLocales.removeAll(checkedLocalesSet);
    for (Iterator i = checkedLocalesSet.iterator(); i.hasNext();) {
      String localeId = (String) i.next();
      journalAppend(SubjectTypes.TYPE_locale, localeId, Actions.ACTION_DEL);
    }
    checkedLocalesSet.clear();
    return RESULT_OK;
  }


  public boolean isLocaleChecked(String locale)
  {
    return checkedLocalesSet.contains(locale);
  }

  public List getLocales()
  {
    if (sort != null && sort.startsWith("-"))
      return reverseLocales;
    else
      return locales;
  }

  public String[] getCheckedLocales()
  {
    return checkedLocales;
  }

  public void setCheckedLocales(String[] checkedLocales)
  {
    this.checkedLocales = checkedLocales;
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

  public String getViewLocaleResource()
  {
    return viewLocaleResource;
  }

  public void setViewLocaleResource(String viewLocaleResource)
  {
    this.viewLocaleResource = viewLocaleResource;
  }

  public String getMbView()
  {
    return mbView;
  }

  public void setMbView(String mbView)
  {
    this.mbView = mbView;
  }
}
