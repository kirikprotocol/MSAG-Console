/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: Nov 28, 2002
 * Time: 6:51:28 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.jsp.smsc.users;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.NullResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.UserQuery;
import ru.novosoft.smsc.admin.users.UserManager;

import java.util.*;

public class Index extends IndexBean
{
	public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
	public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
	public static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

	protected QueryResultSet users = null;

	protected String editUserLogin = null;

	protected String[] checkedUserLogins = new String[0];
	protected Set checkedUserLoginsSet = new HashSet();
	protected String mbAdd = null;
	protected String mbDelete = null;
	protected String mbEdit = null;

	protected UserManager userManager;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		pageSize = preferences.getUsersPageSize();
		if (sort != null)
			preferences.getUsersSortOrder().set(0, sort);
		else
			sort = (String) preferences.getUsersSortOrder().get(0);

		userManager = appContext.getUserManager();

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		users = new NullResultSet();

		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbAdd != null)
			return RESULT_ADD;
		else if (mbEdit != null)
			return RESULT_EDIT;
		else if (mbDelete != null)
		{
			int dresult = deleteUsers();
			if (dresult != RESULT_OK)
				return result;
		}

		logger.debug("Users.Index - process with sorting [" + (String) preferences.getUsersSortOrder().get(0) + "]");
		users = appContext.getUserManager().query(new UserQuery(pageSize, preferences.getUserFilter(), preferences.getUsersSortOrder(), startPosition));
		totalSize = users.getTotalSize();

		checkedUserLoginsSet.addAll(Arrays.asList(checkedUserLogins));

		return RESULT_OK;
	}

	private int deleteUsers()
	{
		return RESULT_OK;
	}

	public boolean isLoginChecked(String login)
	{
      return checkedUserLoginsSet.contains(login);
	}


	/******************************** properties **************************************/
	public QueryResultSet getUsers()
	{
		return users;
	}

	public String[] getCheckedUserLogins()
	{
		return checkedUserLogins;
	}

	public void setCheckedUserLogins(String[] checkedUserLogins)
	{
		this.checkedUserLogins = checkedUserLogins;
	}

	public String getEditUserLogin()
	{
		return editUserLogin;
	}

	public void setEditUserLogin(String editUserLogin)
	{
		this.editUserLogin = editUserLogin;
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
}
