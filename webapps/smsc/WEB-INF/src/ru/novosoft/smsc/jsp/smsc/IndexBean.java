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
	protected String mbFirst = null;
	protected String mbPrev = null;
	protected String mbNext = null;
	protected String mbLast = null;

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbFilter != null)
			return RESULT_FILTER;
		else if (mbFirst != null)
			startPosition = 0;
		else if (mbPrev != null)
			startPosition = startPosition > pageSize ? startPosition - pageSize : 0;
		else if (mbNext != null)
			startPosition += pageSize;
		else if (mbLast != null)
			startPosition = (totalSize / pageSize + (totalSize % pageSize == 0 ? -1 : 0)) * pageSize;

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

	public int getPageSize()
	{
		return pageSize;
	}
}
