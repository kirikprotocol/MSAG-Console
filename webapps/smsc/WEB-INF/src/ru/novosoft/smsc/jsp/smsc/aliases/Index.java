/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */
package ru.novosoft.smsc.jsp.smsc.aliases;

import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.NullResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasQuery;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

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

	public int process(SMSCAppContext appContext, List errors)
	{
		aliases = new NullResultSet();

		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbAdd != null)
			return RESULT_ADD;
		else if (mbEdit != null)
			return RESULT_EDIT;
		else if (mbDelete != null)
		{
			int dresult = deleteAliases();
			if (dresult != RESULT_OK)
				return result;
			else
				return RESULT_DONE;
		}

		logger.debug("Aliases.Index - process with sorting [" + (String) preferences.getAliasesSortOrder().get(0) + "]");
		aliases = smsc.getAliases().query(new AliasQuery(pageSize, preferences.getAliasesFilter(), preferences.getAliasesSortOrder(), startPosition));
		totalSize = aliases.getTotalSize();

		checkedAliasesSet.addAll(Arrays.asList(checkedAliases));

		return RESULT_OK;
	}

	protected int deleteAliases()
	{
		for (int i = 0; i < checkedAliases.length; i++)
		{
			String alias = checkedAliases[i];
			smsc.getAliases().remove(alias);
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

	/******************** properties *************************/

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
