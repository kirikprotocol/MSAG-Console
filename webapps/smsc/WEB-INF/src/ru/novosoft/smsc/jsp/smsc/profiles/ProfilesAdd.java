/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */
package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.jsp.SmscBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;

public class ProfilesAdd extends SmscBean
{
	protected String mbSave = null;
	protected String mbCancel = null;

   protected String mask = ".0.0.0";
	protected byte report = Profile.REPORT_OPTION_None;
	protected byte codepage = Profile.CODEPAGE_Default;

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbCancel != null)
			return RESULT_DONE;
		else if (mbSave != null)
			return save();

		return RESULT_OK;
	}

	protected int save()
	{
		final Mask address = new Mask(mask);
		Profile profile = new Profile(address, codepage, report);
		try
		{
			switch (smsc.updateProfile(address, profile))
			{
				case 1:	//pusUpdated
					return RESULT_DONE;
				case 2: //pusInserted
					return RESULT_DONE;
				case 3: //pusUnchanged
					return error(SMSCErrors.error.profiles.identicalToDefault);
				case 4: //pusError
					return error(SMSCErrors.error.unknown);
				default:
					return error(SMSCErrors.error.unknown);
			}
		}
		catch (AdminException e)
		{
			try
			{
				logger.error("Couldn't add profile [\"" + profile.getMask().getMask() + "\", " + profile.getCodepageString() + ", " + profile.getReportOptionsString() + "]", e);
			}
			catch (AdminException e1)
			{
				logger.error("Couldn't log error about adding profile [\"" + profile.getMask().getMask() + "\"]", e1);
				logger.error("Couldn't add profile [\"" + profile.getMask().getMask() + "\"]", e);
			}
			return error(SMSCErrors.error.profiles.couldntAdd, e);
		}
	}

	/*************************** properties *********************************/

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
}
