package ru.novosoft.smsc.jsp.smsc.smsc_service;

import ru.novosoft.smsc.jsp.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.SortedList;

import java.util.*;
import java.security.Principal;

/**
 * Created by igork
 * Date: 22.05.2003
 * Time: 19:10:03
 */
public class Logging extends PageBean
{
	public final static String catParamNamePrefix = "category_";

	private Map logCategories = null;;
	private SortedList logCategoriesNames = null;
	private String mbSave = null;
	private String mbCancel = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		try
		{
			logCategories = appContext.getSmsc().getLogCategories();
			logCategoriesNames = new SortedList(logCategories.keySet());
		}
		catch (AdminException e)
		{
			logCategories = new HashMap();
			logCategoriesNames = new SortedList(logCategories.keySet());
			return error(SMSCErrors.error.smsc.couldntGetLogCats, e);
		}
		return result;
	}

	public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal, Map parameters)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mbSave != null)
			return save(parameters);
		else if (mbCancel != null)
			return RESULT_DONE;

		return result;
	}

	private int save(Map parameters)
	{
		Map cats = new HashMap();
		for (Iterator i = parameters.entrySet().iterator(); i.hasNext();)
		{
			Map.Entry entry = (Map.Entry) i.next();
			String paramName = (String) entry.getKey();
			if (paramName.startsWith(catParamNamePrefix))
			{
				cats.put(paramName.substring(catParamNamePrefix.length()), getParamValue(entry.getValue()));
				logger.info("cat   param: " + paramName + ":=" + getParamValue(entry.getValue()));
			}
			else
			{
				logger.info("WRONG param: " + paramName + ":=" + getParamValue(entry.getValue()));
			}
		}
		try
		{
			appContext.getSmsc().setLogCategories(cats);
			return RESULT_DONE;
		}
		catch (AdminException e)
		{
			return error(SMSCErrors.error.smsc.couldntSetLogCats, e);
		}
	}

	private String getParamValue(Object value)
	{
		if (value instanceof String)
			return (String) value;
		else if (value instanceof String[])
		{
			String result = "";
			final String[] values = (String[]) value;
			for (int i = 0; i < values.length; i++)
			{
				result += values[i];
			}
			return result;
		}
		else
			return value.toString();
	}

	public List getCategoryNames()
	{
		return logCategoriesNames;
	}

	public String getCategoryPriority(String categoryName)
	{
		return (String) logCategories.get(categoryName);
	}

	public String getMbSave()
	{
		return mbSave;
	}

	public void setMbSave(String mbSave)
	{
		this.mbSave = mbSave;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}
}
