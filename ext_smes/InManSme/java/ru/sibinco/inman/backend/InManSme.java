package ru.sibinco.inman.backend;

import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 29.11.2005
 * Time: 12:08:14
 * To change this template use File | Settings | File Templates.
 */
public class InManSme extends Service
{
    public InManSme(final ServiceInfo info, final String host, final int port) {
        super(info, port);
        this.host = host;
    }

    public void updateInfo(final SMSCAppContext appContext) throws AdminException {
      setInfo(appContext.getHostsManager().getServiceInfo(getInfo().getId()));
    }
}
