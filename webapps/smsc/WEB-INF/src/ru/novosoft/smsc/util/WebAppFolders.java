package ru.novosoft.smsc.util;

import java.io.File;

/**
 * Created by igork
 * Date: Jan 20, 2003
 * Time: 6:41:28 PM
 */
public class WebAppFolders
{
	protected static File webappFolder = null;
	protected static File webinfFolder = null;
	protected static File webinfLibFolder = null;
	protected static File workFolder = null;
	protected static File daemonsFolder = null;

	public static void init(String webappFolder)
	{
		WebAppFolders.webappFolder = new File(webappFolder);
		WebAppFolders.webinfFolder = new File(webappFolder, "WEB-INF");
		WebAppFolders.webinfLibFolder = new File(webinfFolder, "lib");
		WebAppFolders.workFolder = new File(webinfFolder, "work");
		WebAppFolders.daemonsFolder = new File(webinfFolder, "daemons");
	}

	public static File getServiceFolder(String host, String serviceId)
	{
		return new File(new File(daemonsFolder, host), serviceId);
	}

	public static File getServiceJspsFolder(String serviceId)
	{
		return new File(webappFolder, "esme_" + serviceId);
	}


	public static File getWebappFolder()
	{
		return webappFolder;
	}

	public static File getWebinfFolder()
	{
		return webinfFolder;
	}

	public static File getWebinfLibFolder()
	{
		return webinfLibFolder;
	}

	public static File getWorkFolder()
	{
		return workFolder;
	}

	public static File getDaemonsFolder()
	{
		return daemonsFolder;
	}
}
