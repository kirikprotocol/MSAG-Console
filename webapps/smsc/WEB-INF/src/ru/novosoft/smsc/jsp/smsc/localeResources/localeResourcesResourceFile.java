package ru.novosoft.smsc.jsp.smsc.localeResources;

import org.apache.log4j.Category;
import ru.novosoft.smsc.util.WebAppFolders;

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

	public void printLocaleText(Writer out)
	{
		File localeFile = new File(WebAppFolders.getSmscConfFolder(), Index.PREFIX + locale + ".xml");
		if (!localeFile.exists())
		{
			Category.getInstance(this.getClass()).error("cannot find locale resource file \"" + localeFile.getAbsolutePath() + '"');
			return;
		}

		try
		{
			BufferedReader in = new BufferedReader(new InputStreamReader(Thread.currentThread().getContextClassLoader().getResourceAsStream("dtds/locale_resources.dtd")));
			out.write("<?xml version=\"1.0\" encoding=\"windows-1251\"?>\n");
			out.write("<!DOCTYPE locale_resources [\n");
			for (String line = skipHeader(in); line != null; line = in.readLine())
				out.write(line);
			in.close();
			out.write("]>");
			in = new BufferedReader(new FileReader(localeFile));
			for (String line = skipHeader(in); line != null; line = in.readLine())
			{
				out.write(line);
				out.write('\n');
			}
			in.close();
		}
		catch (FileNotFoundException e)
		{
			Category.getInstance(this.getClass()).error("cannot read locale resource file \"" + localeFile.getAbsolutePath() + '"');
		}
		catch (IOException e)
		{
			Category.getInstance(this.getClass()).error("cannot read locale resource file \"" + localeFile.getAbsolutePath() + "\", nested: " + e.getMessage());
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
