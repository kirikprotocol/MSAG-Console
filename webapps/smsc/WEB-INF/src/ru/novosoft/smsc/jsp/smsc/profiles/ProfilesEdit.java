/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:41:31
 */
package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.SmscBean;

import java.util.List;

public class ProfilesEdit extends SmscBean
{
	protected String mbSave = null;
	protected String mbCancel = null;

	protected String mask = null;
	protected byte report = -1;
	protected byte codepage = -1;

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mask == null)
			return error(SMSCErrors.error.profiles.profileNotSpecified);

		if (report == -1 && codepage == -1)
		{
			try
			{
				Profile p = smsc.lookupProfile(new Mask(mask));
				report = p.getReportOptions();
				codepage = p.getCodepage();
			}
			catch (AdminException e)
			{
				logger.error("Couldn't lookup profile \"" + mask + '"', e);
				return error(SMSCErrors.error.profiles.couldntLookup, e);
			}
		}

		if (mbCancel != null)
			return RESULT_DONE;
		else if (mbSave != null)
			return save();

		return RESULT_OK;
	}

	protected int save()
	{
		if (!Mask.isMaskValid(mask))
			return error(SMSCErrors.error.profiles.invalidMask, mask);

		try
		{
			final Mask address = new Mask(mask);
			Profile profile = new Profile(address, codepage, report);
			switch (smsc.updateProfile(address, profile))
			{
				case 1:	//pusUpdated
				case 2: //pusInserted
				case 3: //pusUnchanged
					appContext.getStatuses().setProfilesChanged(true);
					return RESULT_DONE;
				case 4: //pusError
				default:
					return error(SMSCErrors.error.unknown);
			}
		}
		catch (AdminException e)
		{
			logger.error("Couldn't update profile [\"" + mask + "\", " + codepage + ", " + report + "]", e);
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
