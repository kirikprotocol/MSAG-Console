package ru.novosoft.smsc.jsp.smsc.localeResources;

import ru.novosoft.smsc.jsp.*;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.util.jsp.MultipartServletRequest;
import ru.novosoft.util.jsp.MultipartDataSource;

import java.util.List;
import java.util.Arrays;
import java.io.*;

/**
 * Created by igork
 * Date: Feb 18, 2003
 * Time: 4:39:40 PM
 */
public class LocaleResourcesAdd extends PageBean
{
	private String mbCancel = null;;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		return RESULT_OK;
	}

	public int process(MultipartServletRequest multi, SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbCancel != null)
			return RESULT_DONE;

		if (multi != null)
		{
			MultipartDataSource dataFile = null;
			try
			{
				dataFile = multi.getMultipartDataSource("resourceFile");
				if (dataFile == null)
					return error(SMSCErrors.error.localeResources.fileNotAttached);

				if (dataFile.getContentType().equals("text/xml"))
				{
					String name = dataFile.getName();
					int pos = name.lastIndexOf('/');
					if (pos >= 0)
						name = name.substring(pos + 1);
					pos = name.lastIndexOf('\\');
					if (pos >= 0)
						name = name.substring(pos + 1);

					if (!name.matches(Index.PATTERN))
						return error(SMSCErrors.error.localeResources.wrongFileName);

					File file = new File(WebAppFolders.getSmscConfFolder(), name);
					if (file.exists())
						return error(SMSCErrors.error.localeResources.alreadyExists);

					BufferedReader is = new BufferedReader(new InputStreamReader(dataFile.getInputStream()));
					Writer os = new FileWriter(file);
					for (String line = is.readLine(); line != null; line = is.readLine())
					{
						os.write(line);
						os.write('\n');
					}
					is.close();
					os.close();
					dataFile.close();
					dataFile = null;
					return RESULT_DONE;
				}
				else
					return error(SMSCErrors.error.localeResources.wrongFileType);
			}
			catch (Throwable t)
			{
				logger.debug("Couldn't receive file", t);
				return error(SMSCErrors.error.localeResources.couldntReceiveFile, t);
			}
			finally
			{
				if (dataFile != null)
				{
					dataFile.close();
					dataFile = null;
				}
			}
		}
		else
			return RESULT_OK;
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
