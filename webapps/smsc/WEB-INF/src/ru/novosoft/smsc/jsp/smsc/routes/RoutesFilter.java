/*
 * Created by igork
 * Date: 05.11.2002
 * Time: 23:38:40
 */
package ru.novosoft.smsc.jsp.smsc.routes;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.impl.RouteFilter;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.MaskList;

import java.util.*;

public class RoutesFilter extends SmscBean
{
	protected RouteFilter filter = null;

	protected String[] srcChks = null;
	protected String[] srcMasks = null;
	protected String[] dstChks = null;
	protected String[] dstMasks = null;
	protected String[] smeChks = null;

	protected boolean strict = false;
	protected boolean showSrc = false;
	protected boolean showDst = false;

	protected Set srcChksSet = null;
	protected Set dstChksSet = null;
	protected Set smeChksSet = null;

	protected String mbApply = null;
	protected String mbClear = null;
	protected String mbCancel = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		filter = appContext.getUserPreferences().getRoutesFilter();

		if (srcMasks == null || dstMasks == null)
		{
			srcChks = filter.getSourceSubjectNames();
			srcMasks = filter.getSourceMaskStrings();
			dstChks = filter.getDestinationSubjectNames();
			dstMasks = filter.getDestinationMaskStrings();
			smeChks = filter.getSmeIds();

			strict = filter.isIntersection();
			showSrc = appContext.getUserPreferences().isRouteShowSrc();
			showDst = appContext.getUserPreferences().isRouteShowDst();
		}

		if (srcChks == null) srcChks = new String[0];
		if (dstChks == null) dstChks = new String[0];
		if (smeChks == null) smeChks = new String[0];

		srcChks = trimStrings(srcChks);
		try
		{
			srcMasks = MaskList.normalizeMaskList(trimStrings(srcMasks));
		}
		catch (AdminException e)
		{
			return error(SMSCErrors.error.routes.invalidSourceMask, e);
		}
		dstChks = trimStrings(dstChks);
		try
		{
			dstMasks = MaskList.normalizeMaskList(trimStrings(dstMasks));
		}
		catch (AdminException e)
		{
			return error(SMSCErrors.error.routes.invalidDestinationMask, e);
		}
		smeChks = trimStrings(smeChks);

		srcChksSet = new HashSet(Arrays.asList(srcChks));
		dstChksSet = new HashSet(Arrays.asList(dstChks));
		smeChksSet = new HashSet(Arrays.asList(smeChks));

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbApply != null)
		{
			try
			{
				filter.setSourceMaskStrings(srcMasks);
			}
			catch (AdminException e)
			{
				return error(SMSCErrors.error.routes.invalidSourceMask, e);
			}
			try
			{
				filter.setDestinationMaskStrings(dstMasks);
			}
			catch (AdminException e)
			{
				return error(SMSCErrors.error.routes.invalidDestinationMask, e);
			}
			filter.setSourceSubjectNames(srcChks);
			filter.setDestinationSubjectNames(dstChks);
			filter.setSmeIds(smeChks);
			filter.setIntersection(strict);
			appContext.getUserPreferences().setRouteShowSrc(showSrc);
			appContext.getUserPreferences().setRouteShowDst(showDst);
			return RESULT_DONE;
		}
		else if (mbClear != null)
		{
			srcChks = srcMasks = dstChks = dstMasks = smeChks = new String[0];

			strict = showSrc = showDst = false;

			srcChksSet.clear();
			dstChksSet.clear();
			smeChksSet.clear();
			return RESULT_OK;
		}
		else if (mbCancel != null)
			return RESULT_DONE;

		return RESULT_OK;
	}


	public boolean isSrcChecked(String subj)
	{
		return srcChksSet.contains(subj);
	}

	public boolean isDstChecked(String subj)
	{
		return dstChksSet.contains(subj);
	}

	public boolean isSmeChecked(String sme)
	{
		return smeChksSet.contains(sme);
	}

	public Collection getAllSubjects()
	{
		return routeSubjectManager.getSubjects().getNames();
	}

	public Collection getAllSmes()
	{
		return smeManager.getSmeNames();
	}


	/***************************** properties **********************************/
	public String[] getSrcChks()
	{
		return srcChks;
	}

	public void setSrcChks(String[] srcChks)
	{
		this.srcChks = srcChks;
	}

	public String[] getSrcMasks()
	{
		return srcMasks;
	}

	public void setSrcMasks(String[] srcMasks)
	{
		this.srcMasks = srcMasks;
	}

	public String[] getDstChks()
	{
		return dstChks;
	}

	public void setDstChks(String[] dstChks)
	{
		this.dstChks = dstChks;
	}

	public String[] getDstMasks()
	{
		return dstMasks;
	}

	public void setDstMasks(String[] dstMasks)
	{
		this.dstMasks = dstMasks;
	}

	public String[] getSmeChks()
	{
		return smeChks;
	}

	public void setSmeChks(String[] smeChks)
	{
		this.smeChks = smeChks;
	}

	public boolean isStrict()
	{
		return strict;
	}

	public void setStrict(boolean strict)
	{
		this.strict = strict;
	}

	public boolean isShowSrc()
	{
		return showSrc;
	}

	public void setShowSrc(boolean showSrc)
	{
		this.showSrc = showSrc;
	}

	public boolean isShowDst()
	{
		return showDst;
	}

	public void setShowDst(boolean showDst)
	{
		this.showDst = showDst;
	}

	public Set getSmeChksSet()
	{
		return smeChksSet;
	}

	public void setSmeChksSet(Set smeChksSet)
	{
		this.smeChksSet = smeChksSet;
	}

	public String getMbApply()
	{
		return mbApply;
	}

	public void setMbApply(String mbApply)
	{
		this.mbApply = mbApply;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}

	public String getMbClear()
	{
		return mbClear;
	}

	public void setMbClear(String mbClear)
	{
		this.mbClear = mbClear;
	}
}
