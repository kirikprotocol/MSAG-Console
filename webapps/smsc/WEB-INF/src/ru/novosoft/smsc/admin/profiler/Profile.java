/*
 * User: igork
 * Date: 20.08.2002
 * Time: 13:30:24
 */
package ru.novosoft.smsc.admin.profiler;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;

public class Profile
{
	public static final byte CODEPAGE_Default = 0;
	public static final byte CODEPAGE_UCS2 = 8;
	public static final byte REPORT_OPTION_None = 0;
	public static final byte REPORT_OPTION_Full = 1;

	private Mask mask;

	private byte codepage;
	private byte reportOptions;

	/**
	 * Constructs <code>Profile</code> from string returned by SMSC.
	 * @param profileString profile string returned by SMSC. Must be
	 * formatted as <code>&lt;codepage&gt;,&lt;report options&gt;</code>
	 * where &lt;codepage&gt; must be "default" or "UCS2" and &lt;report
	 * options&gt; must be "full" or "none".
	 */
	public Profile(Mask mask, String profileString)
			  throws AdminException
	{
		this.mask = mask;
		int delimPos = profileString.indexOf(',');
		if (delimPos < 1)
			throw new AdminException("profile string returned by SMSC misformatted");
		String codepageString = profileString.substring(0, delimPos);
		String reportoptionsString = profileString.substring(delimPos + 1);

		setCodepage(codepageString);
		setReportOptions(reportoptionsString);
	}

	public Profile(Mask mask, byte codepage, byte reportOptions)
	{
		this.mask = mask;
		this.codepage = codepage;
		this.reportOptions = reportOptions;
	}

	public byte getCodepage()
	{
		return codepage;
	}

	public String getCodepageString()
			  throws AdminException
	{
		switch (codepage)
		{
			case CODEPAGE_Default:
				return "default";
			case CODEPAGE_UCS2:
				return "UCS2";
			default:
				throw new AdminException("Codepage is not initialized");
		}
	}

	public void setCodepage(byte codepage)
	{
		this.codepage = codepage;
	}

	public void setCodepage(String codepageString)
			  throws AdminException
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

	public String getReportOptionsString()
			  throws AdminException
	{
		switch (reportOptions)
		{
			case REPORT_OPTION_Full:
				return "full";
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

	public void setReportOptions(String reportoptionsString)
			  throws AdminException
	{
		if (reportoptionsString.equalsIgnoreCase("full"))
			reportOptions = REPORT_OPTION_Full;
		else if (reportoptionsString.equalsIgnoreCase("none"))
			reportOptions = REPORT_OPTION_None;
		else
			throw new AdminException("Unknown report option: " + reportoptionsString);
	}

	public String getStringRepresentation()
			  throws AdminException
	{
		return getCodepageString() + ',' + getReportOptionsString();
	}

	public Mask getMask()
	{
		return mask;
	}
}
