/*
 * Created by igork
 * Date: 15.11.2002
 * Time: 13:51:58
 */
package ru.novosoft.smsc.jsp.smsc;

import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.List;

public abstract class IndexBean extends SmscBean
{
	public static final int RESULT_FILTER = SmscBean.PRIVATE_RESULT;
	protected static final int PRIVATE_RESULT = SmscBean.PRIVATE_RESULT + 1;

	protected int startPosition = 0;
	protected int totalSize = 0;
	protected int pageSize = 0;

	protected String sort = null;

	protected String mbFilter = null;

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbFilter != null)
			return RESULT_FILTER;

		return RESULT_OK;
	}


	/***************************************** properties **************************************************************/

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

	public void setPageSize(int pageSize)
	{
		this.pageSize = pageSize;
	}

	public String getSort()
	{
		return sort;
	}

	public void setSort(String sort)
	{
		this.sort = sort;
	}

	public String getMbFilter()
	{
		return mbFilter;
	}

	public void setMbFilter(String mbFilter)
	{
		this.mbFilter = mbFilter;
	}

	public int getPageSize()
	{
		return pageSize;
	}

	protected void clear()
	{
		startPosition = totalSize = pageSize = 0;
		sort = null;
		mbFilter = null;
	}
}
