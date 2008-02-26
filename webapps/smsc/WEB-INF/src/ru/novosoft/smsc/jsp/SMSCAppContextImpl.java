package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.fraud.FraudConfigManager;
import ru.novosoft.smsc.admin.region.RegionsManager;
import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.closedgroups.ClosedGroupManager;
import ru.novosoft.smsc.admin.console.Console;
import ru.novosoft.smsc.admin.daemon.DaemonManagerHSImpl;
import ru.novosoft.smsc.admin.daemon.DaemonManagerImpl;
import ru.novosoft.smsc.admin.journal.Journal;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.resource_group.ResourceGroupConstants;
import ru.novosoft.smsc.admin.resource_group.ResourceGroupManager;
import ru.novosoft.smsc.admin.resources.ResourcesManager;
import ru.novosoft.smsc.admin.resources.ResourcesManagerImpl;
import ru.novosoft.smsc.admin.route.RouteSubjectManagerImpl;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.admin.smsc_service.*;
import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.perfmon.PerfServer;
import ru.novosoft.smsc.topmon.TopServer;
import ru.novosoft.smsc.util.LocaleMessages;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.util.jsp.AppContextImpl;
import ru.novosoft.util.menu.TopMenu;

import java.io.File;
import java.security.Principal;
import java.util.*;

public class SMSCAppContextImpl extends AppContextImpl implements SMSCAppContext {
    private Config webappConfig = null;
    private WebXml webXmlConfig = null;
    private HostsManager hostsManager = null;
    private UserManager userManager = null;
    private PerfServer perfServer = null;
    private TopServer topServer = null;
    private SmeManager smeManager = null;
    private RouteSubjectManager routeSubjectManager = null;
    private RegionsManager regionsManager = null;
    private FraudConfigManager fraudConfigManager = null;
    private ResourcesManager resourcesManager = null;
    private ProviderManager providerManager = null;
    private CategoryManager categoryManager = null;
    private AclManager aclManager = null;
    private ServiceManager serviceManager = null;
    private ClosedGroupManager closedGroupManager = null;

    private Smsc smsc = null;
    private SmscList smscList = null;

    private Statuses statuses = new StatusesImpl();
    private Journal journal = new Journal();

    private Console console = null;

    private String initErrorCode = null;

    public SMSCAppContextImpl(String configFileName) {
        super();
        try {
            System.out.println("Starting SMSC Administration Web Application **************************************************");
            webappConfig = new Config(new File(configFileName));
            System.out.println("webappConfig = " + configFileName + " **************************************************");
            WebAppFolders.init(webappConfig.getString("system.webapp folder"), webappConfig.getString("system.work folder"));
            try {
                webXmlConfig = new WebXml(new File(WebAppFolders.getWebinfFolder(), "web.xml"));
            }
            catch (Throwable e) {
                e.printStackTrace();
                System.err.println("Could not load web.xml - administration services access rights can not be changed");
                throw new AdminException("Could not load web.xml - administration services access rights can not be changed");
            }

            try {
                Constants.instType = ResourceGroupConstants.getTypeFromString(webappConfig.getString(ResourceGroupConstants.RESOURCEGROUP_INSTALLTYPE_PARAM_NAME));
                try {
                    if (Constants.instType == ResourceGroupConstants.RESOURCEGROUP_TYPE_HS)
                        Constants.HSMODE_MIRRORFILES_PATH = webappConfig.getString(ResourceGroupConstants.RESOURCEGROUP_MIRRORPATH_PARAM_NAME);
                }
                catch (Exception e) {
                    System.out.println("Installation mirrorpath is not defined in webconfig (installation.mirrorpath missed)");
                    throw new AdminException("Installation mirrorpath is not defined in webconfig");
                }
            }
            catch (Exception e) {
                System.out.println("Installation type is not defined in webconfig (installation.type missed)");
                throw new AdminException("Installation type is not defined in webconfig");
            }
            resourcesManager = new ResourcesManagerImpl();
            smscList = new SmscList(webappConfig, this);
            smeManager = new SmeManagerImpl(smsc);
            serviceManager = new ServiceManagerImpl();
            serviceManager.add(smsc);
            routeSubjectManager = new RouteSubjectManagerImpl(smeManager);
            regionsManager = RegionsManager.getInstance();
            fraudConfigManager = FraudConfigManager.getInstance();
            switch (Constants.instType) {
                case ResourceGroupConstants.RESOURCEGROUP_TYPE_SINGLE:
                    DaemonManagerImpl daemonManagerImpl = new DaemonManagerImpl(smeManager, webappConfig);
                    hostsManager = new HostsManagerSingleImpl(daemonManagerImpl, serviceManager, smeManager, routeSubjectManager);
                    break;
                case ResourceGroupConstants.RESOURCEGROUP_TYPE_HS:
                    DaemonManagerHSImpl daemonManagerHSImpl = new DaemonManagerHSImpl(smeManager, webappConfig);
                    hostsManager = new HostsManagerHSImpl(daemonManagerHSImpl, serviceManager, smeManager, routeSubjectManager);
                    break;
                case ResourceGroupConstants.RESOURCEGROUP_TYPE_HA:
                    WebAppFolders.setHAServicesFolder(webappConfig.getString("system.services folder"));
                    ResourceGroupManager resourceGroupManager = new ResourceGroupManager(this);
                    hostsManager = new HostsManagerHAImpl(resourceGroupManager, serviceManager, smeManager, routeSubjectManager);
                    break;
                default:
                    throw new AdminException("Unknown installation type");
            }
            aclManager = new AclManager(this);
            Config localeConfig = new Config(new File(webappConfig.getString("system.localization file")));
            LocaleMessages.init(localeConfig); // ������ ���������� ������, ��� �����������
            File usersConfig = new File(webappConfig.getString("system.users file"));
            System.out.println("Init UserManager");
            userManager = new UserManager(usersConfig);
            providerManager = new ProviderManager(webappConfig);
            categoryManager = new CategoryManager(webappConfig);
            closedGroupManager = new ClosedGroupManager(this);
            getStatuses().setRoutesSaved(routeSubjectManager.hasSavedConfiguration());
            perfServer = new PerfServer(webappConfig);
            perfServer.setSmscHost(smscList.getSmsc().getHost());
            perfServer.start();
            topServer = new TopServer(webappConfig);
            topServer.setSmscHost(smscList.getSmsc().getHost());
            topServer.start();
            startConsole();
            if (webappConfig.containsParameter("installation.uid"))
              TopMenu.setInstallationId(webappConfig.getString("installation.uid"));
            System.out.println("SMSC Administration Web Application Started  **************************************************");
        }
        catch (Exception e) {
            System.err.println("Exception in initialization:");
            e.printStackTrace();
            initErrorCode = SMSCErrors.error.appContextNotInitialized;
        }
    }

    private void startConsole() throws Exception {
        boolean needConsole = false;
        try {
            needConsole = webappConfig.getBool("console.enabled");
        }
        catch (Exception eee) { /* do nothing, console disabled by default */
        }
        if (needConsole) {
            console = new Console(this);
            console.start();
        }
    }

    public Config getConfig() {
        return webappConfig;
    }

    public HostsManager getHostsManager() {
        return hostsManager;
    }

    public Smsc getSmsc() {
        return smsc;
    }

    public void setSmsc(Smsc newSmsc) {
        smsc = newSmsc;
    }

    public SmeManager getSmeManager() {
        return smeManager;
    }

    public RouteSubjectManager getRouteSubjectManager() {
        return routeSubjectManager;
    }

    public ResourcesManager getResourcesManager() {
        return resourcesManager;
    }

    public ProviderManager getProviderManager() {
        return providerManager;
    }

    public CategoryManager getCategoryManager() {
        return categoryManager;
    }

    public Statuses getStatuses() {
        return statuses;
    }

    public UserManager getUserManager() {
        return userManager;
    }

    HashMap smeContexts = new HashMap();
    Object smeContextsLock = new Object();
    long smeContextsId = 0;

    public Long registerSMEContext(SMEAppContext smeContext) {
        Long contextId = null;
        synchronized (smeContextsLock) {
            contextId = new Long(smeContextsId++);
            smeContexts.put(contextId, smeContext);
        }
        return contextId;
    }

    public void unregisterSMEContext(Long contextId) {
        synchronized (smeContextsLock) {
            if (contextId != null) smeContexts.remove(contextId);
        }
    }

    public void destroy() {
        if (console != null) console.close();
        if (perfServer != null) perfServer.shutdown();
        if (topServer != null) topServer.shutdown();

        synchronized (smeContextsLock) // shutdown all smeContexts
        {
            for (Iterator it = smeContexts.values().iterator(); it.hasNext();) {
                Object obj = it.next();
                if (obj instanceof SMEAppContext) {
                    ((SMEAppContext) obj).shutdown();
                }
            }
            smeContexts.clear();
            smeContextsId = 0;
        }
    }

    public String getLocaleString(Principal principal, String key) {
        String result = null;
        if (userManager != null) {
            result = getLocaleString(userManager.getPrefs(principal).getLocale(), key);
        }
        if (result == null) {
            try {
                ResourceBundle bundle = ResourceBundle.getBundle(LocaleMessages.SMSC_BUNDLE_NAME, new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE));
                result = bundle.getString(key);
            }
            catch (MissingResourceException e) {
                result = /*"ResourceNotFound: " + */key;
            }
        }
        return result;
    }

    private static String getLocaleString(Locale locale, String key) {
        return LocaleMessages.getString(locale, key);
    }

    public Set getLocaleStrings(Principal principal, String prefix) {
        return LocaleMessages.getStrings(userManager.getPrefs(principal).getLocale(), prefix);
    }

    public String getInitErrorCode() {
        return initErrorCode;
    }

    public Journal getJournal() {
        return journal;
    }

    public WebXml getWebXmlConfig() {
        return webXmlConfig;
    }

    public AclManager getAclManager() {
        return aclManager;
    }

    public SmscList getSmscList() {
        return smscList;
    }

    public ServiceManager getServiceManager() {
        return serviceManager;
    }

    public void setTopMonSmscHost(String smscHost) {
        topServer.setSmscHost(smscHost);
    }

    public void setPerfMonSmscHost(String smscHost) {
        perfServer.setSmscHost(smscHost);
    }

    public ClosedGroupManager getClosedGroupManager() {
        return closedGroupManager;
    }

    public RegionsManager getRegionsManager() {
        return regionsManager;
    }

    public FraudConfigManager getFraudConfigManager() {
      return fraudConfigManager;
    }
}

