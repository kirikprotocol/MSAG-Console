package ru.novosoft.smsc.admin.resources;

import ru.novosoft.smsc.util.*;

import java.io.*;
import java.util.List;

/**
 * Created by igork
 * Date: 11.03.2003
 * Time: 17:23:36
 */
public class ResourcesManagerImpl implements ResourcesManager
{
	private File createFile(String localeName)
	{
		return new File(WebAppFolders.getSmscConfFolder(), RESOURCE_FILENAME_PREFIX + localeName + RESOURCE_FILENAME_SUFFIX);
	}

	private String extractLocaleName(String fileName)
	{
		return fileName.substring(RESOURCE_FILENAME_PREFIX_LENGTH, RESOURCE_FILENAME_PREFIX_LENGTH + RESOURCE_FILENAME_BODY_LENGTH);
	}

	public void add(String localeName, InputStream resourceStream) throws IOException
	{
		File file = createFile(localeName);

		File tmpFile = null;
		tmpFile = Functions.saveFileToTemp(resourceStream, "locale_resource_", ".xml.tmp");
		if (file.exists())
			file.delete();
		tmpFile.renameTo(file);
	}

	public boolean add(String localeName, File resourceFile)
	{
		File file = createFile(localeName);
		if (file.exists())
			file.delete();
		return resourceFile.renameTo(file);
	}

	public List list()
	{
		SortedList result = new SortedList();
		File confFolder = WebAppFolders.getSmscConfFolder();
		String[] files = confFolder.list(new FilenameFilter()
		{
			public boolean accept(File dir, String name)
			{
				return name.matches(RESOURCE_FILENAME_PATTERN);
			}
		});
		for (int i = 0; i < files.length; i++)
			result.add(extractLocaleName(files[i]));
		return result;
	}

	public BufferedReader getResource(String localeName) throws FileNotFoundException
	{
		return new BufferedReader(new FileReader(createFile(localeName)));
	}

	public boolean isResourceFileExists(String localeName)
	{
		return createFile(localeName).exists();
	}
}
