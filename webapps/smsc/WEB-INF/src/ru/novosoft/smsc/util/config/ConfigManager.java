/*
 * Created by igork
 * Date: Mar 12, 2002
 * Time: 2:53:22 PM
 */
package ru.novosoft.smsc.util.config;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.xml.Utils;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;



public class ConfigManager
{//toremove whole class
	static protected boolean isInitialized = false;
	static protected String configurationFileName = "config.xml";
	static protected String configurationFileDir = "";
	static protected ConfigManager monitor = null;

	protected Config config = null;


	static public class IsNotInitializedException extends Exception
	{
		IsNotInitializedException(String s)
		{
			super(s);
		}
	}


	static public class WrongParamTypeException extends Exception
	{
		WrongParamTypeException(String s)
		{
			super(s);
		}
	}


	static public void Init(String configFileName)
	{
		configurationFileName = configFileName;

		int slashpos = configurationFileName.lastIndexOf(File.separatorChar);
		if (slashpos < 0)
			configurationFileDir = "";
		else
			configurationFileDir = configurationFileName.substring(0, slashpos);

		isInitialized = true;
	}

	static public ConfigManager getInstance()
			throws IsNotInitializedException, FactoryConfigurationError, ParserConfigurationException, IOException, SAXException
	{
		if (!isInitialized)
			throw new IsNotInitializedException("Configuration Manager is not initialized. Make ConfigManager.Init(...) call before ConfigManager.getInstance()");

		if (monitor == null)
			monitor = new ConfigManager(configurationFileName);

		return monitor;
	}

	protected ConfigManager(String configFileName)
			throws FactoryConfigurationError, ParserConfigurationException, IOException, SAXException
	{
		config = new Config(Utils.parse(new FileReader(configFileName)));
	}

	public Config getConfig()
	{
		return config;
	}

	public void save(String encoding)
			throws IOException, WrongParamTypeException
	{
		SaveableConfigTree tree = new SaveableConfigTree(config);
		File tmpFile = File.createTempFile("smsc_config_", ".xml.tmp", new File(configurationFileDir));
		PrintWriter out = new PrintWriter(new FileWriter(tmpFile));
		Functions.storeConfigHeader(out, "config", "configuration.dtd", encoding);
		//// C++ code doesn't know about other codings // System.getProperty("file.encoding");
		tree.write(out, "  ");
		Functions.storeConfigFooter(out, "config");
		out.flush();
		out.close();

		tmpFile.renameTo(new File(configurationFileName));
	}
}

