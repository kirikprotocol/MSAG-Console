/*
 * Created by igork
 * Date: 04.10.2002
 * Time: 22:27:32
 */
package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.SaveableConfigTree;
import ru.novosoft.smsc.util.xml.Utils;

import java.io.*;
import java.util.*;

public class DBSMEConfig
{
	protected static final String SECTION_StartupLoader = "StartupLoader";
	protected static final String SECTION_StartupLoader_DataSourceDrivers = SECTION_StartupLoader + ".DataSourceDrivers";
	protected static final String SECTION_DBSme = "DBSme";
	protected static final String SECTION_DBSme_ThreadPool = SECTION_DBSme + ".ThreadPool";
	protected static final String SECTION_DBSme_SMSC = SECTION_DBSme + ".SMSC";
	protected static final String SECTION_DBSme_DataProviders = SECTION_DBSme + ".DataProviders";

	protected Config config = null;
	protected File configurationFileName = null;

	public DBSMEConfig(File configurationFileName)
			  throws Exception
	{
		this.configurationFileName = configurationFileName;
		System.out.println("configurationFileName.getCanonicalPath() = " + configurationFileName.getCanonicalPath());
		this.config = new Config(Utils.parse(new FileReader(configurationFileName)));
	}

	public List getDataSourceDrivers()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		List result = new LinkedList();
		for (Iterator i = config.getSectionChildSectionNames(SECTION_StartupLoader_DataSourceDrivers).iterator(); i.hasNext();)
		{
			String driverName = (String) i.next();
			result.add(new DataSourceDriverInfo(config,
															SECTION_StartupLoader_DataSourceDrivers,
															driverName.substring(SECTION_StartupLoader_DataSourceDrivers.length() + 1)));
		}
		return result;
	}

	public void addDataSourceDriver(String name, String type, String loadup)
	{
		String path = SECTION_StartupLoader_DataSourceDrivers + '.' + name;
		config.setString(path + ".type", type);
		config.setString(path + ".loadup", loadup);
	}

	public void removeDataSourceDriver(String name)
	{
		String path = SECTION_StartupLoader_DataSourceDrivers + '.' + name;
		config.removeParam(path + ".type");
		config.removeParam(path + ".loadup");
	}

	public MessageSet getDBSmeMessages()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return new MessageSet(config, SECTION_DBSme);
	}

	public String getSvcType()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(SECTION_DBSme + ".SvcType");
	}

	public void setSvcType(String svcType)
	{
		config.setString(SECTION_DBSme + ".SvcType", svcType);
	}

	public int getProtocolId()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getInt(SECTION_DBSme + ".ProtocolId");
	}

	public void setProtocolId(int protocolId)
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		config.setInt(SECTION_DBSme + ".ProtocolId", protocolId);
	}

	public ThreadPoolInfo getThreadPoolInfo()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return new ThreadPoolInfo(config, SECTION_DBSme_ThreadPool);
	}

	public SMSCInfo getSMSCInfo()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return new SMSCInfo(config, SECTION_DBSme_SMSC);
	}

	public List getProviders()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		List result = new LinkedList();
		Collection names = config.getSectionChildSectionNames(SECTION_DBSme_DataProviders);
		for (Iterator i = names.iterator(); i.hasNext();)
		{
			String providerName = ((String) i.next());
			result.add(new DataProviderInfo(config, SECTION_DBSme_DataProviders, providerName.substring(providerName.lastIndexOf('.') + 1)));
		}
		return result;
	}

	public DataProviderInfo getProvider(String providerName)
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		Collection names = config.getSectionChildSectionNames(SECTION_DBSme_DataProviders);
		for (Iterator i = names.iterator(); i.hasNext();)
		{
			String fullName = (String) i.next();
			String name = fullName.substring(fullName.lastIndexOf('.') + 1);
			if (name.equals(providerName))
				return new DataProviderInfo(config, SECTION_DBSme_DataProviders, name);
		}
		return null;
	}

	public void addProvider(String providerName, String address, String type, int connections,
									String dbInstance, String dbUserName, String dbUserPassword)
	{
		final String fullProviderName = SECTION_DBSme_DataProviders + '.' + providerName;
		config.setString(fullProviderName + ".address", address);
		final String dsPrefix = fullProviderName + ".DataSource";
		config.setString(dsPrefix + ".type", type);
		config.setInt(dsPrefix + ".connections", connections);
		config.setString(dsPrefix + ".dbInstance", dbInstance);
		config.setString(dsPrefix + ".dbUserName", dbUserName);
		config.setString(dsPrefix + ".dbUserPassword", dbUserPassword);
	}

	public void save()
			  throws Exception
	{
		SaveableConfigTree tree = new SaveableConfigTree(config);
		File tmpFile = File.createTempFile("dbsme_config_", ".xml.tmp");
		OutputStream out = new FileOutputStream(tmpFile);
	        String encoding = null; //System.getProperty("file.encoding");
	        if( encoding == null ) encoding = "ISO-8859-1";

		out.write(("<?xml version=\"1.0\" encoding=\""+encoding+"\"?>\n").getBytes());
		out.write("<!DOCTYPE config SYSTEM \"file://configuration.dtd\">\n\n".getBytes());
		out.write("<config>\n".getBytes());
		tree.write(out, "  ");
		out.write("</config>\n".getBytes());
		out.flush();
		out.close();

		tmpFile.renameTo(configurationFileName);
	}

	public void renameProvider(String providerName, String newName)
	{
		config.renameSection(SECTION_DBSme_DataProviders + '.' + providerName, SECTION_DBSme_DataProviders + '.' + newName);
	}
}
