package ru.novosoft.smsc.admin.daemon;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.config.Config;

import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 24.03.2006
 * Time: 15:18:21
 * To change this template use File | Settings | File Templates.
 */
public class DaemonManagerHS {
    private DaemonHS daemon = null;
    private Category logger = Category.getInstance(this.getClass());
    private Config config = null;

    public DaemonManagerHS(SmeManager smeManager, Config config) {
        this.config = config;
        logger.debug("Initializing daemonHS manager");
        try {
            String daemonName = config.getString("daemon.host");
            int port = config.getInt("daemon.port");
            daemon = new DaemonHS(daemonName, port, smeManager, config.getString("daemon.services folder"));
            logger.debug("Daemon \"" + daemonName + ':' + port + "\" added");
        } catch (Config.ParamNotFoundException e) {
            logger.debug("Misconfigured HS daemon", e);
        } catch (Config.WrongParamTypeException e) {
            logger.debug("Misconfigured HS daemon", e);
        }
        logger.debug("Daemon manager initialized");
    }

    public DaemonHS getDaemon() {
        return daemon;
    }

    public synchronized void store() throws Config.WrongParamTypeException, IOException {
        config.removeSection("daemon");
        config.setString("daemon.host", daemon.getHost());
        config.setInt("daemon.port", daemon.getPort());
        config.setString("daemon.services folder", daemon.getDaemonServicesFolder());
        config.save();
    }

    public Map refreshServices(SmeManager smeManager) throws AdminException {
        Map result = new HashMap();
        result.putAll(daemon.refreshServices(smeManager));
        return result;
    }

    public List getHostNames() {
        List list = new SortedList();
        list.add(daemon.getHost());
        return list;
    }

    public void addService(ServiceInfo serviceInfo) throws AdminException {
        daemon.addService(serviceInfo);
    }
}
