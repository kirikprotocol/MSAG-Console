package ru.novosoft.smsc.jsp.smsc.localeResources;

import ru.novosoft.smsc.jsp.*;
import ru.novosoft.smsc.util.WebAppFolders;

import java.io.File;
import java.util.List;

/**
 * Created by igork
 * Date: Feb 18, 2003
 * Time: 2:14:00 PM
 */
public class LocaleResourcesView extends PageBean
{
	private String locale = null;
	private File localeFile = null;
	private String mbDone = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

      if (locale == null || locale.length() != Index.BODY_LENGTH || !locale.matches(Index.BODY_PATTERN))
			return error(SMSCErrors.error.localeResources.incorrectLocaleName, locale == null ? "" : locale);

		localeFile = new File(WebAppFolders.getSmscConfFolder(), Index.PREFIX + locale + ".xml");
		if (!localeFile.exists())
			return error(SMSCErrors.error.localeResources.localeResourcesFileNotFound, locale);

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
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
