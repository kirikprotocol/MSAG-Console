/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */
package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import java.util.List;

public class ProfilesAdd extends ProfilesBean
{
	protected String mbSave = null;
	protected String mbCancel = null;

	public ProfilesAdd()
	{
		report = Profile.REPORT_OPTION_None;
		codepage = Profile.CODEPAGE_Default;
	}

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
		logger.debug("Add new profile: " + mask);
		if (!Mask.isMaskValid(mask))
			return error(SMSCErrors.error.profiles.invalidMask, mask);
		if (!appContext.getSmsc().isLocaleRegistered(locale))
			return error(SMSCErrors.error.profiles.invalidLocale, locale);

		try
		{
			final Mask address = new Mask(mask);
			Profile profile = new Profile(address, codepage, report, locale);
			switch (smsc.updateProfile(address, profile))
			{
				case 1: //pusUpdated
				case 2: //pusInserted
					appContext.getStatuses().setProfilesChanged(true);
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
			logger.error("Couldn't add profile [\"" + mask + "\", " + codepage + ", " + report + ", " + locale + "]", e);
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

}
