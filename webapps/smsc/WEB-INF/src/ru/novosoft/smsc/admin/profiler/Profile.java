/*
 * User: igork
 * Date: 20.08.2002
 * Time: 13:30:24
 */
package ru.novosoft.smsc.admin.profiler;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;

import java.util.StringTokenizer;

public class Profile
{
	public static final byte CODEPAGE_Default = 0;
	public static final byte CODEPAGE_UCS2 = 8;
	public static final byte REPORT_OPTION_None = 0;
	public static final byte REPORT_OPTION_Full = 1;
	public static final byte REPORT_OPTION_Final = 3;

	private Mask mask;

	private byte codepage;
	private byte reportOptions;
	private String locale;

	/**
	 * Constructs <code>Profile</code> from string returned by SMSC.
	 * @param profileString profile string returned by SMSC. Must be
	 * formatted as <code>&lt;codepage&gt;,&lt;report options&gt;</code>
	 * where &lt;codepage&gt; must be "default" or "UCS2" and &lt;report
	 * options&gt; must be "full" or "none".
	 */
	public Profile(Mask mask, String profileString) throws AdminException
	{
		this.mask = mask;
		int i = 0;
		for (StringTokenizer tokenizer = new StringTokenizer(profileString, ",", false); tokenizer.hasMoreTokens();)
		{
			String token = tokenizer.nextToken().trim();
			switch (i++)
			{
				case 0:
					setCodepage(token);
					break;
				case 1:
					setReportOptions(token);
					break;
				case 2:
					setLocale(token);
					break;
			}
		}
		if (i < 3)
			throw new AdminException("profile string returned by SMSC misformatted: " + profileString);
	}

	public Profile(Mask mask, byte codepage, byte reportOptions, String locale)
	{
		this.mask = mask;
		setCodepage(codepage);
		setReportOptions(reportOptions);
		setLocale(locale);
	}

	public Profile(Mask mask, String codepage, String reportOptions, String locale) throws AdminException
	{
		this.mask = mask;
		setCodepage(codepage);
		setReportOptions(reportOptions);
		setLocale(locale);
	}

	public byte getCodepage()
	{
		return codepage;
	}

	public String getCodepageString() throws AdminException
	{
		switch (codepage)
		{
			case CODEPAGE_Default:
				return "default";
			case CODEPAGE_UCS2:
				return "UCS2";
			default:
				throw new AdminException("Unknown codepage: " + codepage);
		}
	}

	public void setCodepage(byte codepage)
	{
		this.codepage = codepage;
	}

	public void setCodepage(String codepageString) throws AdminException
	{
		if (codepageString.equalsIgnoreCase("default"))
			codepage = CODEPAGE_Default;
		else if (codepageString.equalsIgnoreCase("UCS2"))
			codepage = CODEPAGE_UCS2;
		else
			throw new AdminException("Unknown codepage: " + codepageString);
	}

	public byte getReportOptions()
	{
		return reportOptions;
	}

	public String getReportOptionsString() throws AdminException
	{
		switch (reportOptions)
		{
			case REPORT_OPTION_Full:
				return "full";
			case REPORT_OPTION_Final:
				return "final";
			case REPORT_OPTION_None:
				return "none";
			default:
				throw new AdminException("Report option is not initialized");
		}
	}

	public void setReportOptions(byte reportOptions)
	{
		this.reportOptions = reportOptions;
	}

	public void setReportOptions(String reportoptionsString) throws AdminException
	{
		if (reportoptionsString.equalsIgnoreCase("full"))
			reportOptions = REPORT_OPTION_Full;
		else if (reportoptionsString.equalsIgnoreCase("final"))
			reportOptions = REPORT_OPTION_Final;
		else if (reportoptionsString.equalsIgnoreCase("none"))
			reportOptions = REPORT_OPTION_None;
		else
			throw new AdminException("Unknown report option: " + reportoptionsString);
	}

	public String getStringRepresentation() throws AdminException
	{
		return getCodepageString() + ',' + getReportOptionsString() + ',' + locale;
	}

	public Mask getMask()
	{
		return mask;
	}

	public String getLocale()
	{
		return locale;
	}

	public void setLocale(String locale)
	{
		this.locale = locale;
		if (this.locale == null)
			this.locale = "";
	}
}
