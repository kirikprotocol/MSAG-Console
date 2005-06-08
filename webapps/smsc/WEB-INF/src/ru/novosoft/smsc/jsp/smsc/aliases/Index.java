package ru.novosoft.smsc.jsp.smsc.aliases;

/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */

import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasQuery;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.Iterator;

public class Index extends IndexBean
{
  public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
  public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
  public static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

  protected QueryResultSet aliases = null;

  protected String editAlias = null;
  protected String editAddress = null;
  protected boolean editHide = false;

  protected String[] checkedAliases = new String[0];
  protected Set checkedAliasesSet = new HashSet();

  protected String mbAdd = null;
  protected String mbDelete = null;
  protected String mbEdit = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    pageSize = preferences.getAliasesPageSize();
    if (sort != null)
      preferences.getAliasesSortOrder().set(0, sort);
    else
      sort = (String) preferences.getAliasesSortOrder().get(0);

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    aliases = new EmptyResultSet();

    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbAdd != null)
      return RESULT_ADD;
    else if (mbEdit != null)
      return RESULT_EDIT;
    else if (mbDelete != null) {
      int dresult = deleteAliases();
      if (dresult != RESULT_OK)
        return result;
      else
        return RESULT_DONE;
    }

    logger.debug("Aliases.Index - process with sorting [" + (String) preferences.getAliasesSortOrder().get(0) + "]");
    aliases = smsc.getAliases().query(new AliasQuery(pageSize, preferences.getAliasesFilter(), preferences.getAliasesSortOrder(), startPosition));
      if (request.getSession().getAttribute("alias") != null) {
          aliases = getAliasesByAlias((String) request.getSession().getAttribute("alias"));
          request.getSession().removeAttribute("alias");
      }
      totalSize = aliases.getTotalSize();

    checkedAliasesSet.addAll(Arrays.asList(checkedAliases));

    return RESULT_OK;
  }

    private QueryResultSet getAliasesByAlias(String alias) {
        boolean found = false;
        QueryResultSet aliases = null;
        while (!found) {
            aliases = smsc.getAliases().query(new AliasQuery(pageSize, preferences.getAliasesFilter(), preferences.getAliasesSortOrder(), startPosition));
            for (Iterator i = aliases.iterator(); i.hasNext();) {
                DataItem item = (DataItem) i.next();
                String al = (String) item.getValue("Alias");
                if (al.equals(alias)) {
                    found = true;
                }
            }
            if (!found) {
                startPosition += pageSize;
            }
        }
        return aliases;
    }

    protected int deleteAliases()
  {
    for (int i = 0; i < checkedAliases.length; i++) {
      String alias = checkedAliases[i];
      smsc.getAliases().remove(alias);
      journalAppend(SubjectTypes.TYPE_alias, alias, Actions.ACTION_DEL);
      appContext.getStatuses().setAliasesChanged(true);
    }
    checkedAliases = new String[0];
    checkedAliasesSet.clear();
    return RESULT_OK;
  }

  public boolean isAliasChecked(String alias)
  {
    return checkedAliasesSet.contains(alias);
  }

  /**
   * ***************** properties ************************
   */

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

  public QueryResultSet getAliases()
  {
    return aliases;
  }

  public void setAliases(QueryResultSet aliases)
  {
    this.aliases = aliases;
  }

  public String getEditAlias()
  {
    return editAlias;
  }

  public void setEditAlias(String editAlias)
  {
    this.editAlias = editAlias;
  }

  public String getEditAddress()
  {
    return editAddress;
  }

  public void setEditAddress(String editAddress)
  {
    this.editAddress = editAddress;
  }

  public boolean isEditHide()
  {
    return editHide;
  }

  public void setEditHide(boolean editHide)
  {
    this.editHide = editHide;
  }

  public String[] getCheckedAliases()
  {
    return checkedAliases;
  }

  public void setCheckedAliases(String[] checkedAliases)
  {
    this.checkedAliases = checkedAliases;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

}
