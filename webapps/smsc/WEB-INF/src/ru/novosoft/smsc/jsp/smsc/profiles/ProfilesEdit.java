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

import java.util.List;

public class ProfilesEdit extends ProfilesBean {
	protected String mbSave = null;
	protected String mbCancel = null;

	public ProfilesEdit()
	{
		report = -1;
		codepage = -1;
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		if (mbCancel != null)
			return RESULT_DONE;

		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mask == null)
			return error(SMSCErrors.error.profiles.profileNotSpecified);

		if (report == -1 && codepage == -1) {
			try {
				Profile p = smsc.profileLookup(new Mask(mask));
				report = p.getReportOptions();
				codepage = p.getCodepage();
				locale = p.getLocale();
				aliasHide = p.isAliasHide();
				aliasModifiable = p.isAliasModifiable();
			} catch (AdminException e) {
				logger.error("Couldn't lookup profile \"" + mask + '"', e);
				return error(SMSCErrors.error.profiles.couldntLookup, mask, e);
			}
		}

		if (mbSave != null)
			return save();

		return RESULT_OK;
	}

	protected int save()
	{
		if (!Mask.isMaskValid(mask))
			return error(SMSCErrors.error.profiles.invalidMask, mask);

		try {
			final Mask address = new Mask(mask);
			Profile profile = new Profile(address, codepage, report, locale, aliasHide, aliasModifiable);
			switch (smsc.profileUpdate(address, profile)) {
				case 1: //pusUpdated
				case 2: //pusInserted
				case 3: //pusUnchanged
					appContext.getStatuses().setProfilesChanged(true);
					return RESULT_DONE;
				case 4: //pusError
				default:
					return error(SMSCErrors.error.unknown);
			}
		} catch (AdminException e) {
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
}
