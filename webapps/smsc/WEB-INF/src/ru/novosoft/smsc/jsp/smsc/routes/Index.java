/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */
package ru.novosoft.smsc.jsp.smsc.routes;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.NullResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.route.RouteQuery;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class Index extends IndexBean
{
	public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT;
	public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
	public static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

	protected QueryResultSet routes = null;

	protected String editRouteId = null;

	protected String[] checkedRouteIds = new String[0];
	protected Set checkedRouteIdsSet = new HashSet();

	protected String mbAdd = null;
	protected String mbDelete = null;
	protected String mbEdit = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		pageSize = preferences.getRoutesPageSize();
		if (sort != null)
			preferences.getRoutesSortOrder().set(0, sort);
		else
			sort = (String) preferences.getRoutesSortOrder().get(0);

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		routes = new NullResultSet();

		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbAdd != null)
			return RESULT_ADD;
		else if (mbEdit != null)
			return RESULT_EDIT;
		else if (mbDelete != null)
		{
			int dresult = deleteRoutes();
			if (dresult != RESULT_OK)
				return result;
			else
				return RESULT_DONE;
		}

		logger.debug("Routes.Index - process with sorting [" + (String) preferences.getRoutesSortOrder().get(0) + "]");
		routes = routeSubjectManager.getRoutes().query(new RouteQuery(pageSize, preferences.getRoutesFilter(), preferences.getRoutesSortOrder(), startPosition));
		totalSize = routes.getTotalSize();

		checkedRouteIdsSet.addAll(Arrays.asList(checkedRouteIds));

		return RESULT_OK;
	}

	protected int deleteRoutes()
	{
		for (int i = 0; i < checkedRouteIds.length; i++)
		{
			String alias = checkedRouteIds[i];
			routeSubjectManager.getRoutes().remove(alias);
			appContext.getStatuses().setRoutesChanged(true);
		}
		checkedRouteIds = new String[0];
		checkedRouteIdsSet.clear();
		return RESULT_OK;
	}

	public boolean isRouteChecked(String alias)
	{
		return checkedRouteIdsSet.contains(alias);
	}


	public QueryResultSet getRoutes()
	{
		return routes;
	}

	/******************** properties *************************/
	public String getEditRouteId()
	{
		return editRouteId;
	}

	public void setEditRouteId(String editRouteId)
	{
		this.editRouteId = editRouteId;
	}

	public String[] getCheckedRouteIds()
	{
		return checkedRouteIds;
	}

	public void setCheckedRouteIds(String[] checkedRouteIds)
	{
		this.checkedRouteIds = checkedRouteIds;
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
