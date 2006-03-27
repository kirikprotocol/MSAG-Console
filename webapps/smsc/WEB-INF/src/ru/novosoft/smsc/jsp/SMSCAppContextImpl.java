package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.closedgroups.ClosedGroupManager;
import ru.novosoft.smsc.admin.console.Console;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.daemon.DaemonManagerHS;
import ru.novosoft.smsc.admin.journal.Journal;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.resource_group.ResourceGroupConstants;
import ru.novosoft.smsc.admin.resource_group.ResourceGroupManager;
import ru.novosoft.smsc.admin.resources.ResourcesManager;
import ru.novosoft.smsc.admin.resources.ResourcesManagerImpl;
import ru.novosoft.smsc.admin.route.RouteSubjectManagerImpl;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.admin.smsc_service.*;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.perfmon.PerfServer;
import ru.novosoft.smsc.topmon.TopServer;
import ru.novosoft.smsc.util.LocaleMessages;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.util.jsp.AppContextImpl;

import java.io.File;
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
    private byte instType = ResourceGroupConstants.RESOURCEGROUP_TYPE_UNKNOWN;

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
                System.err.println("Could not load web.xml - administration services access rights can not be changed");
                throw new AdminException("Could not load web.xml - administration services access rights can not be changed");
            }

            resourcesManager = new ResourcesManagerImpl();
            try {
                this.instType = ResourceGroupConstants.getTypeFromString(webappConfig.getString(ResourceGroupConstants.RESOURCEGROUP_INSTALLTYPE_PARAM_NAME));
            }
            catch (Exception e) {
                System.out.println("Installation type is not defined in webconfig (installation.type missed)");
                throw new AdminException("Installation type is not defined in webconfig");
            }
            smscList = new SmscList(webappConfig, this);
            smeManager = new SmeManagerImpl(smsc);
            serviceManager = new ServiceManagerImpl();
            serviceManager.add(smsc);
            routeSubjectManager = new RouteSubjectManagerImpl(smeManager);
            switch (this.instType) {
                case ResourceGroupConstants.RESOURCEGROUP_TYPE_SINGLE:
                    DaemonManager daemonManager = new DaemonManager(smeManager, webappConfig);
                    hostsManager = new HostsManagerSingleImpl(daemonManager, serviceManager, smeManager, routeSubjectManager);
                    break;
                case ResourceGroupConstants.RESOURCEGROUP_TYPE_HS:
                    DaemonManagerHS daemonManagerHS = new DaemonManagerHS(smeManager, webappConfig);
                    hostsManager = new HostsManagerHSImpl(daemonManagerHS, serviceManager, smeManager, routeSubjectManager);
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

    public Locale getLocale() {
        Locale result = null;
        if (userManager != null) {
            User user = userManager.getLoginedUser();
            if (user != null) result = user.getPrefs().getLocale();
        }
        if (result == null) result = new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE);
        return result;
    }

    public String getLocaleString(String key) {
        String result = null;
        if (userManager != null) // �� ������ ������
        {
            User user = userManager.getLoginedUser();
            if (user != null) result = getLocaleString(user.getPrefs().getLocale(), key);
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

    private String getLocaleString(Locale locale, String key) {
        return LocaleMessages.getString(locale, key);
    }

    public Set getLocaleStrings(String prefix) {
        return getLocaleStrings(userManager.getLoginedUser().getPrefs().getLocale(), prefix);
    }


    private Set getLocaleStrings(Locale locale, String prefix) {
        return LocaleMessages.getStrings(locale, prefix);
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

    public byte getInstallType() {
        return instType;
    }

    public ClosedGroupManager getClosedGroupManager() {
        return closedGroupManager;
    }
}

