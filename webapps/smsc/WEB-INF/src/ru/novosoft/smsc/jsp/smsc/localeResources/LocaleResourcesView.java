package ru.novosoft.smsc.jsp.smsc.localeResources;

import ru.novosoft.smsc.admin.resources.ResourcesManager;
import ru.novosoft.smsc.jsp.*;

import java.util.List;

/**
 * Created by igork
 * Date: Feb 18, 2003
 * Time: 2:14:00 PM
 */
public class LocaleResourcesView extends PageBean
{
	private String locale = null;
	private String mbDone = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (locale == null || locale.length() != ResourcesManager.RESOURCE_FILENAME_BODY_LENGTH || !locale.matches(ResourcesManager.RESOURCE_FILENAME_BODY_PATTERN))
			return error(SMSCErrors.error.localeResources.incorrectLocaleName, locale == null ? "" : locale);

		if (!appContext.getResourcesManager().isResourceFileExists(locale))
			return error(SMSCErrors.error.localeResources.localeResourcesFileNotFound, locale);

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mbDone != null)
			return RESULT_DONE;
		return RESULT_OK;
	}

	public String getLocale()
	{
		return locale;
	}

	public void setLocale(String locale)
	{
		this.locale = locale;
	}

	public String getMbDone()
	{
		return mbDone;
	}

	public void setMbDone(String mbDone)
	{
		this.mbDone = mbDone;
	}
}
