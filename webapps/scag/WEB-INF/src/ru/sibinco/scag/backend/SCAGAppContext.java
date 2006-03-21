package ru.sibinco.scag.backend;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.scag.backend.resources.ResourceManager;
import ru.sibinco.scag.backend.routing.ScagRoutingManager;
import ru.sibinco.scag.backend.users.UserManager;
import ru.sibinco.scag.backend.protocol.journal.Journal;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.backend.sme.ProviderManager;
import ru.sibinco.scag.backend.sme.CategoryManager;
import ru.sibinco.scag.backend.daemon.Daemon;
import ru.sibinco.scag.backend.daemon.ServiceInfo;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.backend.operators.OperatorManager;
import ru.sibinco.scag.perfmon.PerfServer;
import ru.sibinco.scag.svcmon.SvcMonServer;
import ru.sibinco.scag.scmon.ScServer;
import ru.sibinco.tomcat_auth.XmlAuthenticator;

import javax.sql.DataSource;
import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;


/**
 * Created by IntelliJ IDEA. User: igork Date: 25.02.2004 Time: 17:22:47
 */
public class SCAGAppContext {
    private static SCAGAppContext instance = null;

    private Logger logger = Logger.getLogger(this.getClass());

    private final Config config;
    private final Config gwConfig;
    private final Config idsConfig;
    private final UserManager userManager;
    private final SmppManager smppManager;
    private final RuleManager ruleManager;
    private final ProviderManager providerManager;
    private final ServiceProvidersManager serviceProvidersManager;
    private final OperatorManager operatorManager;
    private final CategoryManager categoryManager;
    private final ScagRoutingManager scagRoutingManager;
    private final ResourceManager resourceManager;
    private final StatusManager statusManager;
    private final PerfServer perfServer;
    private final SvcMonServer svcMonServer;
    private final ScServer scServer;
    private final Daemon scagDaemon;
    private final Scag scag;
    private final Statuses statuses;
    private final DataSource connectionPool;
    private Journal journal = new Journal();
    private static File scagConfFolder = null;

    private SCAGAppContext(final String config_filename) throws Throwable, ParserConfigurationException, SAXException, Config.WrongParamTypeException,
            Config.ParamNotFoundException, SibincoException {
        try {
            System.out.println("  **  config file:" + new File(config_filename).getAbsolutePath());
            System.out.flush();
            config = new Config(new File(config_filename));
            gwConfig = new Config(new File(config.getString("gw_config")));
            idsConfig = new Config(new File(config.getString("ids_file")));
            String gwDaemonHost = config.getString("gw daemon.host");
            String gwConfigFolder = config.getString("gw_config_folder");
            scagConfFolder = new File(gwConfigFolder);
            connectionPool = null;
            userManager = new UserManager(config.getString("users_config_file"));
            providerManager = new ProviderManager(idsConfig);
            serviceProvidersManager = new ServiceProvidersManager(config.getString("services_file"));
            serviceProvidersManager.init();
            operatorManager = new OperatorManager(config.getString("operators_file"));
            operatorManager.init();
            categoryManager = new CategoryManager(idsConfig);
            smppManager = new SmppManager(config.getString("smpp_file"), providerManager);
            smppManager.init();
            String rulesFolder = config.getString("rules_folder");
            String xsdFolder = config.getString("xsd_folder");
            String xslFolder = config.getString("xsl_folder");
            resourceManager = new ResourceManager(scagConfFolder);
            scagDaemon = new Daemon(gwDaemonHost, (int) config.getInt("gw daemon.port"), smppManager, config.getString("gw daemon.folder"));
            final ServiceInfo scagServiceInfo = (ServiceInfo) scagDaemon.getServices().get(config.getString("gw name"));
            if (scagServiceInfo != null) {
                scag = new Scag(scagServiceInfo, (int) gwConfig.getInt("admin.port"));
            } else {
                scag = new Scag(gwDaemonHost, (int) gwConfig.getInt("admin.port"));
            }
            ruleManager = new RuleManager(new File(rulesFolder), new File(xsdFolder), new File(xslFolder), scag);
            ruleManager.init();
            scagRoutingManager = new ScagRoutingManager(scagConfFolder, smppManager, providerManager, categoryManager);
            scagRoutingManager.init();
            String statusFolder = config.getString("status_folder");
            statusManager = new StatusManager(new File(statusFolder));
            statusManager.init();
            statuses = new Statuses();
            perfServer = new PerfServer(config);
            perfServer.start();
            svcMonServer = new SvcMonServer(config);
            svcMonServer.start();
            scServer = new ScServer(config);
            scServer.start();
            XmlAuthenticator.init(new File(config.getString("users_config_file")));
        } catch (Throwable e) {
            logger.fatal("Could not initialize App Context", e);
            e.printStackTrace();
            throw e;
        }
    }

    public void destroy() {
        perfServer.shutdown();
        svcMonServer.shutdown();
        scServer.shutdown();
    }

    public static synchronized SCAGAppContext getInstance(final String config_filename) throws Throwable, IOException, ParserConfigurationException,
            Config.ParamNotFoundException, SAXException, SibincoException {

        if (null != instance) {
            return instance;
        } else {
            instance = new SCAGAppContext(config_filename);
            return instance;
        }
    }

    public RuleManager getRuleManager() {
        return ruleManager;
    }

    public Config getConfig() {
        return config;
    }

    public Config getGwConfig() {
        return gwConfig;
    }

    public Config getIdsConfig() {
        return idsConfig;
    }

    public UserManager getUserManager() {
        return userManager;
    }

    public ProviderManager getProviderManager() {
        return providerManager;
    }

    public ServiceProvidersManager getServiceProviderManager() {
        return serviceProvidersManager;
    }

    public OperatorManager getOperatorManager() {
        return operatorManager;
    }

    public CategoryManager getCategoryManager() {
        return categoryManager;
    }

    public ScagRoutingManager getScagRoutingManager() {
        return scagRoutingManager;
    }

    public ResourceManager getResourceManager() {
        return resourceManager;
    }

    public StatusManager getStatusManager() {
        return statusManager;
    }

    public Daemon getScagDaemon() {
        return scagDaemon;
    }

    public Scag getScag() {
        return scag;
    }

    public Statuses getStatuses() {
        return statuses;
    }

    public DataSource getDataSource() {
        return connectionPool;
    }

    public Journal getJournal() {
        return journal;
    }

    public SmppManager getSmppManager() {
        return smppManager;
    }

    public static File getScagConfFolder() {
        return scagConfFolder;
    }
}
