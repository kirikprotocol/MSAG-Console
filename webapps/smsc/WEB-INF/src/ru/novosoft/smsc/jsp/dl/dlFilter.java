package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.jsp.*;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;
import java.util.Arrays;

/**
 * Created by igork
 * Date: 13.03.2003
 * Time: 21:26:27
 */
public class dlFilter extends PageBean
{
	private String[] names = null;

	private String mbCancel = null;
	private String mbApply = null;
	private String mbClear = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (names == null)
			names = appContext.getUserPreferences().getDlFilter().getNames();

		names = trimStrings(names);

		return result;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbCancel != null)
			return RESULT_DONE;
		if (mbApply != null)
			return apply();
		if (mbClear != null)
			return clear();

		return result;
	}

	private int clear()
	{
		names = new String[0];
		return RESULT_OK;
	}

	private int apply()
	{
		appContext.getUserPreferences().getDlFilter().setNames(names);
		return RESULT_DONE;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}

	public String getMbApply()
	{
		return mbApply;
	}

	public void setMbApply(String mbApply)
	{
		this.mbApply = mbApply;
	}

	public String getMbClear()
	{
		return mbClear;
	}

	public void setMbClear(String mbClear)
	{
		this.mbClear = mbClear;
	}

	public String[] getNames()
	{
		return names;
	}

	public void setNames(String[] names)
	{
		this.names = names;
	}
}
