package ru.novosoft.smsc.admin.daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.config.Config;

import java.io.IOException;
import java.util.Map;
import java.util.List;

public interface DaemonManager {

    void store() throws AdminException, Config.WrongParamTypeException, IOException;

    Map refreshServices(SmeManager smeManager) throws AdminException;

    Daemon get(String host) throws AdminException;

    List getHostNames();

    void addService(ServiceInfo serviceInfo) throws AdminException;

    Daemon getServiceDaemon(String serviceId) throws AdminException;
}
