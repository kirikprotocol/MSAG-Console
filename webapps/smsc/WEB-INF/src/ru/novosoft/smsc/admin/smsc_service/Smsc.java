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
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.admin.utli.Proxy;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.ProfileDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.ProfileQuery;
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
	private ConfigManager configManager = null;

	private SMEList smes = null;
	private RouteList routes = null;
	private SubjectList subjects = null;
	private AliasSet aliases = null;
	private ProfileDataSource profileDataSource = null;

	private Category logger = Category.getInstance(this.getClass());

	public Smsc(ConfigManager configManager, NSConnectionPool connectionPool)
			throws AdminException, Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		super(new ServiceInfo(Constants.SMSC_SME_ID,
									 configManager.getConfig().getString("smsc.host"),
									 configManager.getConfig().getInt("smsc.port"),
									 "", null, ServiceInfo.STATUS_STOPPED));

		this.configManager = configManager;

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
			final File smscConfFolder = getSmscConfFolder();
			Document smesDoc = Utils.parse(new BufferedReader(new FileReader(new File(smscConfFolder, "sme.xml"))));
			Document routesDoc = Utils.parse(new BufferedReader(new FileReader(new File(smscConfFolder, "routes.xml"))));
			Document aliasesDoc = Utils.parse(new BufferedReader(new FileReader(new File(smscConfFolder, "aliases.xml"))));
			smes = new SMEList(smesDoc.getDocumentElement());
			subjects = new SubjectList(routesDoc.getDocumentElement(), smes);
			routes = new RouteList(routesDoc.getDocumentElement(), subjects, smes);
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
	}

	public RouteList getRoutes()
	{
		return routes;
	}

	public SMEList getSmes()
	{
		return smes;
	}

	public SubjectList getSubjects()
	{
		return subjects;
	}

	private File getSmscConfFolder()
			throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return new File(configManager.getConfig().getString("system.webapp folder"), "WEB-INF/smsc/conf");
	}

	protected PrintWriter storeSmes(PrintWriter out)
	{
		String encoding = null; // C++ code doesn't know about other codings // System.getProperty("file.encoding");
		if (encoding == null) encoding = "ISO-8859-1";
		out.println("<?xml version=\"1.0\" encoding=\"" + encoding + "\"?>");
		out.println("<!DOCTYPE records SYSTEM \"file://SmeRecords.dtd\">");
		out.println();
		out.println("<records>");

		smes.store(out);

		out.println("</records>");
		return out;
	}

	protected PrintWriter storeRoutes(PrintWriter out)
	{
		String encoding = null; // C++ code doesn't know about other codings // System.getProperty("file.encoding");
		if (encoding == null) encoding = "ISO-8859-1";
		out.println("<?xml version=\"1.0\" encoding=\"" + encoding + "\"?>");
		out.println("<!DOCTYPE routes SYSTEM \"file://routes.dtd\">");
		out.println();
		out.println("<routes>");
		subjects.store(out);
		routes.store(out);
		out.println("</routes>");
		return out;
	}

	protected PrintWriter storeAliases(PrintWriter out)
	{
		String encoding = null; // C++ code doesn't know about other codings // System.getProperty("file.encoding");
		if (encoding == null) encoding = "ISO-8859-1";
		out.println("<?xml version=\"1.0\" encoding=\"" + encoding + "\"?>");
		out.println("<!DOCTYPE aliases SYSTEM \"file://AliasRecords.dtd\">");
		out.println();
		out.println("<aliases>");
		aliases.store(out);
		out.println("</aliases>");
		return out;
	}

	public void applyRoutes()
			throws AdminException
	{
		checkComponents();
		saveSmesConfig();
		saveRoutesConfig();
		call(smsc_component, apply_routes_method, Type.Types[Type.StringType], new HashMap());
	}

	public void applyProfiles()
			throws AdminException
	{
		/* todo applyProfiles */
	}

	public void applyAliases()
			throws AdminException
	{
		checkComponents();
		try
		{
			final File smscConfFolder = getSmscConfFolder();

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

			call(smsc_component, apply_aliases_method, Type.Types[Type.StringType], new HashMap());
		}
		catch (Config.ParamNotFoundException e)
		{
			throw new AdminException("Couldn't apply_routes new settings: Administration application misconfigured: " + e.getMessage());
		}
		catch (Config.WrongParamTypeException e)
		{
			throw new AdminException("Couldn't apply_routes new settings: Administration application misconfigured: " + e.getMessage());
		}
		catch (FileNotFoundException e)
		{
			throw new AdminException("Couldn't apply_routes new settings: Couldn't write to destination config file: " + e.getMessage());
		}
	}

	public AliasSet getAliases()
	{
		return aliases;
	}

	public Profile lookupProfile(Mask mask)
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

	public int updateProfile(Mask mask, Profile newProfile)
			throws AdminException
	{
		checkComponents();
		HashMap args = new HashMap();
		args.put("address", mask.getMask());
		args.put("profile", newProfile.getStringRepresentation());
		return ((Long) call(smsc_component, update_profile_method, Type.Types[Type.IntType], args)).intValue();
	}

	public QueryResultSet queryProfiles(ProfileQuery query)
			throws AdminException
	{
		return profileDataSource.query(query);
	}

	public void saveSmesConfig()
			throws AdminException
	{
		try
		{
			final File smscConfFolder = getSmscConfFolder();
			storeSmes(new PrintWriter(new FileOutputStream(new File(smscConfFolder, "sme.xml")), true)).close();
		}
		catch (Config.ParamNotFoundException e)
		{
			throw new AdminException("Couldn't save new smes settings: Administration application misconfigured: " + e.getMessage());
		}
		catch (Config.WrongParamTypeException e)
		{
			throw new AdminException("Couldn't save new smes settings: Administration application misconfigured: " + e.getMessage());
		}
		catch (FileNotFoundException e)
		{
			throw new AdminException("Couldn't save new smes settings: Couldn't write to destination config file: " + e.getMessage());
		}
	}

	public void saveRoutesConfig()
			throws AdminException
	{
		try
		{
			final File smscConfFolder = getSmscConfFolder();
			storeRoutes(new PrintWriter(new FileOutputStream(new File(smscConfFolder, "routes.xml")), true)).close();
		}
		catch (Config.ParamNotFoundException e)
		{
			throw new AdminException("Couldn't save new routes settings: Administration application misconfigured: " + e.getMessage());
		}
		catch (Config.WrongParamTypeException e)
		{
			throw new AdminException("Couldn't save new routes settings: Administration application misconfigured: " + e.getMessage());
		}
		catch (FileNotFoundException e)
		{
			throw new AdminException("Couldn't save new routes settings: Couldn't write to destination config file: " + e.getMessage());
		}
	}

	public void processCancelMessages(Collection messageIds)
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
		params.put("cancelMessageIds", ids + "; " + srcs + "; " + dsts);
		call(smsc_component, process_cancel_messages_method, Type.Types[Type.StringType], params);
	}

	public void flushStatistics()
			throws AdminException
	{
		checkComponents();
		call(smsc_component, flush_statistics_method, Type.Types[Type.StringType], new HashMap());
	}

	public void start(Daemon smscDaemon)
			throws AdminException
	{
		smscDaemon.startService(Constants.SMSC_SME_ID);
	}

	public void stop(Daemon smscDaemon) throws AdminException
	{
		smscDaemon.shutdownService(Constants.SMSC_SME_ID);
	}

	protected void checkComponents() throws AdminException
	{
		if (apply_aliases_method == null || apply_routes_method == null || lookup_profile_method == null
				|| update_profile_method == null || flush_statistics_method == null
				|| process_cancel_messages_method == null)
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
		}
	}

	public Config getSmscConfig()
	{
		try
		{
			File confFile = new File(getSmscConfFolder(), "config.xml");
			Document confDoc = Utils.parse(new FileInputStream(confFile));
			return new Config(confDoc);
		}
		catch (Throwable t)
		{
			logger.error("Couldn't get SMSC config", t);
			return null;
		}
	}

	private void saveSmscConfig(Config config) throws AdminException
	{
		try
		{
			SaveableConfigTree tree = new SaveableConfigTree(config);
			File tmpFile = File.createTempFile("smsc_config_", ".xml.tmp", getSmscConfFolder());
			PrintWriter out = new PrintWriter(new FileWriter(tmpFile));
			String encoding = null; // C++ code doesn't know about other codings // System.getProperty("file.encoding");
			if (encoding == null) encoding = "ISO-8859-1";
			out.println("<?xml version=\"1.0\" encoding=\"" + encoding + "\"?>");
			out.println("<!DOCTYPE config SYSTEM \"file://configuration.dtd\">");
			out.println("");
			out.println("<config>");
			tree.write(out, "  ");
			out.println("</config>");
			out.flush();
			out.close();

			tmpFile.renameTo(new File(getSmscConfFolder(), "config.xml"));
		}
		catch (Throwable t)
		{
			logger.error("Couldn't store SMSC config", t);
			throw new AdminException("Couldn't store SMSC config: " + t.getMessage());
		}
	}

	public void applyConfig(Config config)
			throws AdminException
	{
		checkComponents();
		saveSmscConfig(config);
		if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
			call(smsc_component, apply_smsc_config_method, Type.Types[Type.StringType], new HashMap());
	}
}
