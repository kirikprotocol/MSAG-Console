/*
 * Author: igork
 * Date: 27.05.2002
 * Time: 18:59:55
 */
package ru.novosoft.smsc.admin.smsc_service;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.DistributionListManager;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.admin.utli.Proxy;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.profile.ProfileDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.profile.ProfileQuery;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.*;
import ru.novosoft.smsc.util.xml.Utils;
import ru.novosoft.util.conpool.NSConnectionPool;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;


public class Smsc extends Service
{
	private Component smsc_component = null;
	private Method apply_routes_method = null;
	private Method apply_aliases_method = null;
	private Method lookup_profile_method = null;
	private Method update_profile_method = null;
	private Method flush_statistics_method = null;
	private Method process_cancel_messages_method = null;
	private Method apply_smsc_config_method = null;
	private Method apply_services_method = null;

	private Method msc_registrate_method = null;
	private Method msc_unregister_method = null;
	private Method msc_block_method = null;
	private Method msc_clear_method = null;
	private Method msc_list_method = null;

	private SmeManager smeManager = null;
	private RouteSubjectManager routeSubjectManager;
	private DistributionListAdmin distributionListAdmin = null;

	private AliasSet aliases = null;
	private ProfileDataSource profileDataSource = null;

	private Category logger = Category.getInstance(this.getClass());

	public Smsc(ConfigManager configManager, NSConnectionPool connectionPool, SmeManager smeManager, RouteSubjectManager routeSubjectManager)
			throws AdminException, Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		super(new ServiceInfo(Constants.SMSC_SME_ID,
									 configManager.getConfig().getString("smsc.host"),
									 configManager.getConfig().getInt("smsc.port"),
									 "", null, ServiceInfo.STATUS_STOPPED));

		this.smeManager = smeManager;
		this.routeSubjectManager = routeSubjectManager;

		try
		{
			refreshComponents();
		}
		catch (AdminException e)
		{
			logger.error("Couldn't connect to SMSC", e);
		}

		if (getStatus() == Proxy.StatusConnected)
			checkComponents();

		try
		{
			final File smscConfFolder = WebAppFolders.getSmscConfFolder();
			Document aliasesDoc = Utils.parse(new FileReader(new File(smscConfFolder, "aliases.xml")));
			aliases = new AliasSet(aliasesDoc.getDocumentElement());
			profileDataSource = new ProfileDataSource(connectionPool);
		}
		catch (FactoryConfigurationError error)
		{
			logger.error("Couldn't configure xml parser factory", error);
			throw new AdminException("Couldn't configure xml parser factory: " + error.getMessage());
		}
		catch (ParserConfigurationException e)
		{
			logger.error("Couldn't configure xml parser", e);
			throw new AdminException("Couldn't configure xml parser: " + e.getMessage());
		}
		catch (SAXException e)
		{
			logger.error("Couldn't parse", e);
			throw new AdminException("Couldn't parse: " + e.getMessage());
		}
		catch (IOException e)
		{
			logger.error("Couldn't read", e);
			throw new AdminException("Couldn't read: " + e.getMessage());
		}
		catch (NullPointerException e)
		{
			logger.error("Couldn't parse", e);
			throw new AdminException("Couldn't parse: " + e.getMessage());
		}

		distributionListAdmin = new DistributionListManager(connectionPool);
	}

	protected PrintWriter storeAliases(PrintWriter out)
	{
		Functions.storeConfigHeader(out, "aliases", "AliasRecords.dtd");
		aliases.store(out);
		Functions.storeConfigFooter(out, "aliases");
		return out;
	}

	public synchronized void applyRoutes()
			throws AdminException
	{
		checkComponents();
		smeManager.save();
		routeSubjectManager.save();
		if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
			call(smsc_component, apply_routes_method, Type.Types[Type.StringType], new HashMap());
	}

	public synchronized void applyServices()
			throws AdminException
	{
		checkComponents();
		smeManager.save();
		routeSubjectManager.save();
		if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
			call(smsc_component, apply_services_method, Type.Types[Type.StringType], new HashMap());
	}

	public synchronized void applyAliases()
			throws AdminException
	{
		checkComponents();
		try
		{
			final File smscConfFolder = WebAppFolders.getSmscConfFolder();

			final File aliasConfigFile = new File(smscConfFolder, "aliases.xml");
			try
			{
				logger.debug("Storing alias config to \"" + aliasConfigFile.getCanonicalPath() + '"');
			}
			catch (IOException e)
			{
				logger.debug("Storing alias config to \"" + aliasConfigFile.getName() + "\", and couldn't get canonical path of this file...");
			}
			storeAliases(new PrintWriter(new FileOutputStream(aliasConfigFile), true)).close();

			if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
				call(smsc_component, apply_aliases_method, Type.Types[Type.StringType], new HashMap());
			else
				logger.debug("Couldn't call apply method on SMSC - SMSC is not running. Status is " + getInfo().getStatusStr() + " (" + getInfo().getStatus() + ")");
		}
		catch (FileNotFoundException e)
		{
			throw new AdminException("Couldn't apply_routes new settings: Couldn't write to destination config file: " + e.getMessage());
		}
	}

	public synchronized AliasSet getAliases()
	{
		return aliases;
	}

	public synchronized Profile lookupProfile(Mask mask)
			throws AdminException
	{
		checkComponents();
		HashMap args = new HashMap();
		args.put("address", mask.getMask());
		Object result = call(smsc_component, lookup_profile_method, Type.Types[Type.StringType], args);
		if (result instanceof String)
			return new Profile(mask, (String) result);
		else
			throw new AdminException("Error in response");
	}

	public synchronized int updateProfile(Mask mask, Profile newProfile)
			throws AdminException
	{
		checkComponents();
		HashMap args = new HashMap();
		args.put("address", mask.getMask());
		args.put("profile", newProfile.getStringRepresentation());
		return ((Long) call(smsc_component, update_profile_method, Type.Types[Type.IntType], args)).intValue();
	}

	public synchronized QueryResultSet queryProfiles(ProfileQuery query)
			throws AdminException
	{
		return profileDataSource.query(query);
	}

	public synchronized void applyProfiles()
	{
		// nothing to do
	}

	public synchronized void processCancelMessages(Collection messageIds)
			throws AdminException
	{
		checkComponents();
		String ids = "";
		String srcs = "";
		String dsts = "";
		for (Iterator i = messageIds.iterator(); i.hasNext();)
		{
			CancelMessageData data = (CancelMessageData) i.next();
			ids += data.getMessageId() + (i.hasNext() ? ", " : "");
			srcs += data.getSourceAddress() + (i.hasNext() ? ", " : "");
			dsts += data.getDestinationAddress() + (i.hasNext() ? ", " : "");
		}
		Map params = new HashMap();
		params.put("ids", ids);
		params.put("sources", srcs);
		params.put("destinations", dsts);
		call(smsc_component, process_cancel_messages_method, Type.Types[Type.StringType], params);
	}

	public synchronized void flushStatistics()
			throws AdminException
	{
		checkComponents();
		call(smsc_component, flush_statistics_method, Type.Types[Type.StringType], new HashMap());
	}

	protected void checkComponents()
	{
		if (apply_aliases_method == null || apply_routes_method == null || lookup_profile_method == null
				|| update_profile_method == null || flush_statistics_method == null
				|| process_cancel_messages_method == null
				|| msc_registrate_method == null || msc_unregister_method == null || msc_block_method == null
				|| msc_clear_method == null || msc_list_method == null)
		{
			try
			{
				refreshComponents();
				smsc_component = (Component) getInfo().getComponents().get("SMSC");
				apply_routes_method = (Method) smsc_component.getMethods().get("apply_routes");
				apply_aliases_method = (Method) smsc_component.getMethods().get("apply_aliases");
				lookup_profile_method = (Method) smsc_component.getMethods().get("lookup_profile");
				update_profile_method = (Method) smsc_component.getMethods().get("update_profile");
				flush_statistics_method = (Method) smsc_component.getMethods().get("flush_statistics");
				process_cancel_messages_method = (Method) smsc_component.getMethods().get("process_cancel_messages");
				apply_smsc_config_method = (Method) smsc_component.getMethods().get("apply_smsc_config");
				apply_services_method = (Method) smsc_component.getMethods().get("apply_services");

				msc_registrate_method = (Method) smsc_component.getMethods().get("msc_registrate");
				msc_unregister_method = (Method) smsc_component.getMethods().get("msc_unregister");
				msc_block_method = (Method) smsc_component.getMethods().get("msc_block");
				msc_clear_method = (Method) smsc_component.getMethods().get("msc_clear");
				msc_list_method = (Method) smsc_component.getMethods().get("msc_list");

			}
			catch (AdminException e)
			{
				logger.error("Couldn't check components", e);
			}
		}
	}

	public synchronized Config getSmscConfig()
	{
		try
		{
			File confFile = new File(WebAppFolders.getSmscConfFolder(), "config.xml");
			Document confDoc = Utils.parse(new FileReader(confFile));
			return new Config(confDoc);
		}
		catch (Throwable t)
		{
			logger.error("Couldn't get SMSC config", t);
			return null;
		}
	}

	public synchronized void saveSmscConfig(Config config) throws AdminException
	{
		try
		{
			SaveableConfigTree tree = new SaveableConfigTree(config);
			File tmpFile = File.createTempFile("smsc_config_", ".xml.tmp", WebAppFolders.getSmscConfFolder());
			PrintWriter out = new PrintWriter(new FileWriter(tmpFile));
			Functions.storeConfigHeader(out, "config", "configuration.dtd");
			tree.write(out, "  ");
			Functions.storeConfigFooter(out, "config");
			out.flush();
			out.close();

			tmpFile.renameTo(new File(WebAppFolders.getSmscConfFolder(), "config.xml"));
		}
		catch (Throwable t)
		{
			logger.error("Couldn't store SMSC config", t);
			throw new AdminException("Couldn't store SMSC config: " + t.getMessage());
		}
	}

	public synchronized void applyConfig()
			throws AdminException
	{
		checkComponents();
		call(smsc_component, apply_smsc_config_method, Type.Types[Type.StringType], new HashMap());
	}

	public synchronized void mscRegistrate(String msc) throws AdminException
	{
		checkComponents();
		HashMap args = new HashMap();
		args.put("msc", msc);
		call(smsc_component, msc_registrate_method, Type.Types[Type.StringType], args);
	}

	public synchronized void mscUnregister(String msc) throws AdminException
	{
		checkComponents();
		HashMap args = new HashMap();
		args.put("msc", msc);
		call(smsc_component, msc_unregister_method, Type.Types[Type.StringType], args);
	}

	public synchronized void mscBlock(String msc) throws AdminException
	{
		checkComponents();
		HashMap args = new HashMap();
		args.put("msc", msc);
		call(smsc_component, msc_block_method, Type.Types[Type.StringType], args);
	}

	public synchronized void mscClear(String msc) throws AdminException
	{
		checkComponents();
		HashMap args = new HashMap();
		args.put("msc", msc);
		call(smsc_component, msc_clear_method, Type.Types[Type.StringType], args);
	}

	public synchronized List mscList() throws AdminException
	{
		checkComponents();
		Object result = call(smsc_component, msc_list_method, Type.Types[Type.StringListType], new HashMap());
		if (result instanceof List)
			return (List) result;
		else
			throw new AdminException("Error in response");
	}

	public synchronized boolean isLocaleRegistered(String locale)
	{
		Config config = getSmscConfig();
		String localesString = null;
		try
		{
			localesString = config.getString("core.locales");
		}
		catch (Config.ParamNotFoundException e)
		{
			logger.warn("isLocaleRegistered: Parameter core.locales not found");
			return false;
		}
		catch (Config.WrongParamTypeException e)
		{
			logger.warn("isLocaleRegistered: Parameter core.locales is not string");
			return false;
		}
		return localesString.matches(".*\\b"+locale+"\\b.*");
	}

	public DistributionListAdmin getDistributionListAdmin()
	{
		return distributionListAdmin;
	}
}
