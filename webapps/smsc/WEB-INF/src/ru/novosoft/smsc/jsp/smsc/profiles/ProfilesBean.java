package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.admin.profiler.Profile;

/**
 * Created by igork
 * Date: 11.03.2003
 * Time: 18:38:02
 */
public class ProfilesBean extends SmscBean
{
	protected String mask = "";
	protected byte report = Profile.REPORT_OPTION_None;
	protected byte codepage = Profile.CODEPAGE_Default;
	protected String locale = "";

	public String getMask()
	{
		return mask;
	}

	public void setMask(String mask)
	{
		this.mask = mask;
	}

	public byte getReport()
	{
		return report;
	}

	public void setReport(byte report)
	{
		this.report = report;
	}

	public byte getCodepage()
	{
		return codepage;
	}

	public void setCodepage(byte codepage)
	{
		this.codepage = codepage;
	}

	public String getLocale()
	{
		return locale;
	}

	public void setLocale(String locale)
	{
		this.locale = locale;
	}
}
