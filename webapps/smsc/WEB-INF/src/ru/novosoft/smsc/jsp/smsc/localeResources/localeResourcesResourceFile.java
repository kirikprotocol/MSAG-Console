package ru.novosoft.smsc.jsp.smsc.localeResources;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.io.*;

/**
 * Created by igork
 * Date: Feb 18, 2003
 * Time: 3:07:38 PM
 */
public class localeResourcesResourceFile
{
	private String locale = null;

	private String skipHeader(BufferedReader in) throws IOException
	{
		String line = in.readLine();
		while (line != null && (line.startsWith("<?xml") || line.startsWith("<!DOCTYPE")))
			line = in.readLine();
		return line;
	}

	public void printLocaleText(SMSCAppContext appContext, Writer out)
	{
		try
		{
			BufferedReader inDtd = new BufferedReader(new InputStreamReader(Thread.currentThread().getContextClassLoader().getResourceAsStream("dtds/locale_resources.dtd")));
			BufferedReader inLocale = appContext.getResourcesManager().getResource(locale);
			out.write("<?xml version=\"1.0\" encoding=\"windows-1251\"?>\n");
			out.write("<!DOCTYPE locale_resources [\n");
			for (String line = skipHeader(inDtd); line != null; line = inDtd.readLine())
				out.write(line);
			inDtd.close();
			out.write("]>");
			for (String line = skipHeader(inLocale); line != null; line = inLocale.readLine())
			{
				out.write(line);
				out.write('\n');
			}
			inLocale.close();
		}
		catch (FileNotFoundException e)
		{
			Category.getInstance(this.getClass()).error("cannot find locale resource file for locale \"" + locale + '"', e);
		}
		catch (IOException e)
		{
			Category.getInstance(this.getClass()).error("cannot read locale resource file for locale \"" + locale + "\", nested: " + e.getMessage());
		}
	}

	public String getLocale()
	{
		return locale;
	}

	public void setLocale(String locale)
	{
		this.locale = locale;
	}
}
