/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */
package ru.novosoft.smsc.jsp.smsc.routes;

import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.RouteQuery;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class Index extends SmscBean
{
	public static final int RESULT_FILTER = PRIVATE_RESULT;
	public static final int RESULT_ADD = PRIVATE_RESULT + 1;
	public static final int RESULT_EDIT = PRIVATE_RESULT + 2;

	protected QueryResultSet routes = null;

	protected int startPosition = 0;
	protected int totalSize = 0;

	protected String editRouteId = null;

	protected String[] checkedRouteIds = new String[0];
	protected Set checkedRouteIdsSet = new HashSet();

	protected String mbAdd = null;
	protected String mbDelete = null;
	protected String mbEdit = null;
	protected String mbFilter = null;
	protected String mbFirst = null;
	protected String mbPrev = null;
	protected String mbNext = null;
	protected String mbLast = null;

	protected String sort = null;


	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		UserPreferences up = appContext.getUserPreferences();
		int pageSize = up.getRoutesPageSize();
		if (sort != null)
			up.getRoutesSortOrder().set(0, sort);
		else
			sort = (String) up.getRoutesSortOrder().get(0);

		if (mbAdd != null)
			return RESULT_ADD;
		else if (mbEdit != null)
			return RESULT_EDIT;
		else if (mbFilter != null)
			return RESULT_FILTER;
		else if (mbDelete != null)
		{
			int dresult = deleteRoutes();
			if (dresult != RESULT_OK)
				return result;
		}
		else if (mbFirst != null)
			startPosition = 0;
		else if (mbPrev != null)
			startPosition = startPosition > pageSize ? startPosition - pageSize : 0;
		else if (mbNext != null)
			startPosition += pageSize;
		else if (mbLast != null)
			startPosition = (totalSize / pageSize + (totalSize % pageSize == 0 ? -1 : 0)) * pageSize;

		logger.debug("Aliases.Index - process with sorting [" + (String) up.getAliasesSortOrder().get(0) + "]");
		routes = smsc.getRoutes().query(new RouteQuery(pageSize, up.getRoutesFilter(), up.getRoutesSortOrder(), startPosition));
		totalSize = routes.getTotalSize();

		checkedRouteIdsSet.addAll(Arrays.asList(checkedRouteIds));

		return RESULT_OK;
	}

	protected int deleteRoutes()
	{
		for (int i = 0; i < checkedRouteIds.length; i++)
		{
			String alias = checkedRouteIds[i];
			smsc.getRoutes().remove(alias);
			appContext.getStatuses().setRoutesChanged(true);
		}
		checkedRouteIds = new String[0];
		checkedRouteIdsSet.clear();
		return RESULT_OK;
	}

	public boolean isFirst()
	{
		return startPosition == 0;
	}

	public boolean isLast()
	{
		return routes.isLast();
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
	public int getStartPosition()
	{
		return startPosition;
	}

	public void setStartPosition(int startPosition)
	{
		this.startPosition = startPosition;
	}

	public int getTotalSize()
	{
		return totalSize;
	}

	public void setTotalSize(int totalSize)
	{
		this.totalSize = totalSize;
	}

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

	public String getMbFilter()
	{
		return mbFilter;
	}

	public void setMbFilter(String mbFilter)
	{
		this.mbFilter = mbFilter;
	}

	public String getMbFirst()
	{
		return mbFirst;
	}

	public void setMbFirst(String mbFirst)
	{
		this.mbFirst = mbFirst;
	}

	public String getMbPrev()
	{
		return mbPrev;
	}

	public void setMbPrev(String mbPrev)
	{
		this.mbPrev = mbPrev;
	}

	public String getMbNext()
	{
		return mbNext;
	}

	public void setMbNext(String mbNext)
	{
		this.mbNext = mbNext;
	}

	public String getMbLast()
	{
		return mbLast;
	}

	public void setMbLast(String mbLast)
	{
		this.mbLast = mbLast;
	}

	public String getSort()
	{
		return sort;
	}

	public void setSort(String sort)
	{
		this.sort = sort;
	}
}
