/*
 * Created by igork
 * Date: 05.11.2002
 * Time: 23:38:40
 */
package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.util.tables.impl.ProfileFilter;

import java.util.List;

public class ProfilesFilter extends SmscBean
{
	protected ProfileFilter filter = null;

	protected String[] masks = null;
	protected MaskList maskList = null;
	protected byte codepage = -2;
	protected byte reportinfo = -2;

	protected String mbApply = null;
	protected String mbCancel = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		filter = appContext.getUserPreferences().getProfilesFilter();

		if (codepage == -2 && reportinfo == -2)
		{
			masks = (String[]) filter.getMasks().getNames().toArray(new String[0]);
			codepage = filter.getCodepage();
			reportinfo = filter.getReportinfo();
		}
		if (masks == null)
			masks = new String[0];

		maskList = new MaskList(masks);
		masks = (String[]) maskList.getNames().toArray(new String[0]);

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbApply != null)
		{
			filter.setMasks(maskList);
			filter.setCodepage(codepage);
			filter.setReportinfo(reportinfo);
			return RESULT_DONE;
		}
		else if (mbCancel != null)
			return RESULT_DONE;

		return RESULT_OK;
	}


	/***************************** properties **********************************/
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

	public String[] getMasks()
	{
		return masks;
	}

	public void setMasks(String[] masks)
	{
		this.masks = masks;
	}

	public byte getCodepage()
	{
		return codepage;
	}

	public void setCodepage(byte codepage)
	{
		this.codepage = codepage;
	}

	public byte getReportinfo()
	{
		return reportinfo;
	}

	public void setReportinfo(byte reportinfo)
	{
		this.reportinfo = reportinfo;
	}
}
