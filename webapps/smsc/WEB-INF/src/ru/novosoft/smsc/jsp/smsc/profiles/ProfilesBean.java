package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.util.config.Config;

import java.util.LinkedList;
import java.util.List;

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
	protected List registeredLocales = new LinkedList();

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		try
		{
			registeredLocales = smsc.getRegisteredLocales();
		}
		catch (Throwable e)
		{
			logger.error("Couldn't get registered profiles", e);
			return error(SMSCErrors.error.profiles.couldntGetRegisteredLocales, e);
		}
		return result;
	}

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

	public List getRegisteredLocales()
	{
		return registeredLocales;
	}
}
