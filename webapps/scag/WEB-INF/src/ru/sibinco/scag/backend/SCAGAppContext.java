package ru.sibinco.scag.backend;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;
import ru.sibinco.WHOISDIntegrator.TariffMatrixManager;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.scag.backend.daemon.Daemon;
import ru.sibinco.scag.backend.daemon.ServiceInfo;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.scag.backend.gw.ConfigManager;
import ru.sibinco.scag.backend.gw.logging.LoggingManager;
import ru.sibinco.scag.backend.installation.HSDaemon;
import ru.sibinco.scag.backend.operators.OperatorManager;
import ru.sibinco.scag.backend.protocol.journal.Journal;
import ru.sibinco.scag.backend.resources.ResourceManager;
import ru.sibinco.scag.backend.routing.ScagRoutingManager;
import ru.sibinco.scag.backend.routing.http.HttpRoutingManager;
import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.backend.rules.LiveConnect;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.backend.sme.CategoryManager;
import ru.sibinco.scag.backend.sme.ProviderManager;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.users.UserManager;
import ru.sibinco.scag.backend.stat.counters.CountersManager;
import ru.sibinco.scag.perfmon.PerfServer;
import ru.sibinco.scag.scmon.ScServer;
import ru.sibinco.scag.svcmon.SvcMonServer;
import ru.sibinco.scag.util.LocaleManager;
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
    private final HttpRoutingManager httpRoutingManager;
    private final ResourceManager resourceManager;
    private final LocaleManager localeManager;
    private final ConfigManager configManager;
    private final TariffMatrixManager tMatrixManager;
    private final PerfServer perfServer;
    private final SvcMonServer svcMonServer;
    private final ScServer scServer;
    private final Daemon scagDaemon;
    private final Scag scag;
    private final HSDaemon hsDaemon;
    private final Statuses statuses;
    private final DataSource connectionPool;
    private final LoggingManager loggingManager;
    private final CountersManager countersManager;

    private final LiveConnect liveConnect;
    private Journal journal = new Journal();
    private static File scagConfFolder = null;

    private boolean isCluster = false;

    public boolean isCluster() {
        return isCluster;
    }

    private SCAGAppContext(final String config_filename) throws Throwable, ParserConfigurationException, SAXException, Config.WrongParamTypeException,
            Config.ParamNotFoundException, SibincoException {
        try {
            System.out.println("  **  config file:" + new File(config_filename).getAbsolutePath());
            System.out.flush();
            config = new Config(new File(config_filename));
            String gwConfigFolder = config.getString("gw location.gw_config_folder");
            scagConfFolder = new File(gwConfigFolder);
            gwConfigFolder = gwConfigFolder  + File.separatorChar;
            String gwConfigFile = gwConfigFolder + File.separatorChar + config.getString("gw location.gw_config");
            gwConfig = new Config(new File(gwConfigFile));
            idsConfig = new Config(new File(config.getString("ids_file")));
            configManager = new ConfigManager(gwConfigFile,gwConfig);
            tMatrixManager = new TariffMatrixManager();
            String gwDaemonHost = config.getString("gw daemon.host");
            connectionPool = null;

            String instType = config.getString("installation.type");
            System.out.println("  ***  instType='" + instType + "'" );
            if( instType.equals(HSDaemon.typeCluster) ) { isCluster = true; }

            hsDaemon = new HSDaemon(config.getString("installation.type"), config.getString("installation.mirrorpath"));
            loggingManager = new LoggingManager(config.getString("logger.properties_file"),hsDaemon);
            userManager = new UserManager(config.getString("users_config_file"),hsDaemon);
            providerManager = new ProviderManager(idsConfig);
            serviceProvidersManager = new ServiceProvidersManager(gwConfigFolder + config.getString("gw location.services_file"));
            serviceProvidersManager.init();
            operatorManager = new OperatorManager(gwConfigFolder + config.getString("gw location.operators_file"));
            operatorManager.init();
            categoryManager = new CategoryManager(idsConfig);
            smppManager = new SmppManager(gwConfigFolder + config.getString("gw location.smpp_file"), providerManager);
            smppManager.init();
            String rulesFolder = gwConfigFolder + config.getString("gw location.rules_folder");
            String xsdFolder = gwConfigFolder + config.getString("gw location.xsd_folder");
            String xslFolder = gwConfigFolder + config.getString("gw location.xsl_folder");
            resourceManager = new ResourceManager(scagConfFolder);
            localeManager = new LocaleManager(config.getString("locales_file"));
            countersManager = new CountersManager(config.getString("counters_config_file"), hsDaemon);
            
//            scagDaemon = new Daemon(gwDaemonHost, (int) config.getInt("gw daemon.port"), smppManager, config.getString("gw daemon.folder"));
//            final ServiceInfo scagServiceInfo = (ServiceInfo) scagDaemon.getServices().get(config.getString("gw name"));

            ServiceInfo scagServiceInfo = null;
            if( instType.equals(HSDaemon.typeCluster) ) { ;
//                scagDaemon = new Daemon(gwDaemonHost, (int) config.getInt("gw daemon.port"), smppManager, config.getString("gw daemon.folder"));
                scagDaemon = null;
                scagServiceInfo = null;
            }else{
                scagDaemon = new Daemon(gwDaemonHost, (int) config.getInt("gw daemon.port"), smppManager, config.getString("gw daemon.folder"));
                scagServiceInfo = (ServiceInfo) scagDaemon.getServices().get(config.getString("gw name"));
            }

            if (scagServiceInfo != null) {
                scag = new Scag(scagServiceInfo, (int) gwConfig.getInt("admin.port"));
            } else {
                if( isCluster() ){
                    scag = new Scag(gwDaemonHost, (int) gwConfig.getInt("admin.port"));
                } else {
                    scag = new Scag(gwDaemonHost, (int) gwConfig.getInt("admin.port"));
                }
            }
            ruleManager = new RuleManager(new File(rulesFolder), new File(xsdFolder), new File(xslFolder), scag, hsDaemon);
            ruleManager.init();
            scagRoutingManager = new ScagRoutingManager(scagConfFolder, smppManager, serviceProvidersManager, hsDaemon);
            scagRoutingManager.init();
            httpRoutingManager = new HttpRoutingManager(scagConfFolder, serviceProvidersManager, hsDaemon);
            httpRoutingManager.init();
            StatusManager.getInstance().init(config.getString("status_folder"), config.getString("show_interval"));
            statuses = new Statuses();
            perfServer = new PerfServer(config);
            perfServer.start();
            svcMonServer = new SvcMonServer(config);
            svcMonServer.start();
            scServer = new ScServer(config);
            scServer.start();
            liveConnect = new LiveConnect(ruleManager,config);
            liveConnect.start();
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
        liveConnect.stopLC();
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

    public LoggingManager getLoggingManager() {
        return loggingManager;
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

    public CountersManager getCountersManager() {
        return countersManager;
    }

    public ScagRoutingManager getScagRoutingManager() {
        return scagRoutingManager;
    }

    public HttpRoutingManager getHttpRoutingManager() {
        return httpRoutingManager;
    }

    public ResourceManager getResourceManager() {
        return resourceManager;
    }

    public LocaleManager getLocaleManager() {
        return localeManager;
    }

    public ConfigManager getConfigManager() {
        return configManager;
    }

    public TariffMatrixManager getTMatrixManager() {
      return tMatrixManager;
    }

    public Daemon getScagDaemon() {
        return scagDaemon;
    }

    public Scag getScag() {
        return scag;
    }

    public HSDaemon getHSDaemon() {
        return hsDaemon;
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

    public LiveConnect getLiveConnect() {
      return liveConnect;
    }
  
    public static File getScagConfFolder() {
        return scagConfFolder;
    }
}
