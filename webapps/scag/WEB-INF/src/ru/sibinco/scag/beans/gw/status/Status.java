package ru.sibinco.scag.beans.gw.status;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.beans.SCAGBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.backend.daemon.ServiceInfo;
import ru.sibinco.scag.backend.daemon.Daemon;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;


/**
 * Created by igork Date: 27.07.2004 Time: 17:17:59
 */
public class Status extends SCAGBean {

    private byte scagStatus = ServiceInfo.STATUS_UNKNOWN;

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException 
    {
        //logger.debug("Status.process() start");
        super.process(request, response);
        if( !getAppContext().isCluster() ){
            final Daemon scagDaemon = appContext.getScagDaemon();
            try {
                logger.debug( "Status.process() refreshServices()" );
                scagDaemon.refreshServices(appContext.getSmppManager());
                logger.debug( "Status.process() appContext.getScag().getId()='" + appContext.getScag().getId() + "'" );
                Object gwService = scagDaemon.getServiceInfo( appContext.getScag().getId() );
                logger.debug("Status.process() scagDaemon.getServiceInfo()='" + gwService +"'" );
                if (gwService instanceof ServiceInfo) {
                    logger.debug("Status.process() (gwService instanceof ServiceInfo)" );
                    final ServiceInfo info = (ServiceInfo) gwService;
                    scagStatus = info.getStatus();
                    logger.debug("Status.process() SCAGstatus='" + scagStatus +"'");
                }
            } catch (SibincoException e) {
                logger.warn("Could not refresh services. Details: " + e.getMessage());
            } catch (NullPointerException e) {
                logger.warn("Could not get SCAG daemon");
            }
        }else{
            //logger.debug( "Status.process() CLUSTER" );
//            scagStatus = ServiceInfo.STATUS_CLUSTER;
        }
    }

    public byte getScagStatus() {
        return scagStatus;
    }
}
