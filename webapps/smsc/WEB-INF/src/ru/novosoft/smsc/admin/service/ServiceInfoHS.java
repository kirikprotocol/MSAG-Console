package ru.novosoft.smsc.admin.service;

import org.w3c.dom.Element;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.admin.smsc_service.SmscList;

public class ServiceInfoHS extends ServiceInfo {
    public ServiceInfoHS(final Element serviceElement, final String serviceHost, final SmeManager smeManager, final String daemonServicesFolder)
            throws AdminException {
        super(serviceElement, serviceHost, smeManager, daemonServicesFolder);
        if (status == STATUS_ONLINE) {
            String node = serviceElement.getAttribute("node");
            switch (SmscList.getNodeId(node)) {
                case STATUS_ONLINE1:
                    status = STATUS_ONLINE1;
                    break;
                case STATUS_ONLINE2:
                    status = STATUS_ONLINE2;
                    break;
                default:
                    status = STATUS_UNKNOWN;
                    break;
            }
        }
    }
}
