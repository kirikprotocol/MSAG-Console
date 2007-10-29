package ru.sibinco.otasme;

import ru.sibinco.smsc.utils.smscenters.SmsCentersList;
import ru.sibinco.smsc.utils.smscenters.SmsCentersListException;
import ru.sibinco.otasme.utils.Service;
import ru.sibinco.otasme.utils.ConnectionPoolFactory;
import ru.sibinco.otasme.network.smpp.SMPPMultiplexor;
import ru.sibinco.otasme.engine.service.SubscriptionStateMachine;
import ru.sibinco.otasme.engine.SMPPWorker;
import com.eyeline.sme.utils.worker.IterativeWorker;
import com.eyeline.sme.utils.config.properties.PropertiesConfig;

/**
 * User: artem
 * Date: Oct 8, 2007
 */

public class NewSme {
  public static void main(String[] args) {
    try {
      SmeProperties.init();

      final SmsCentersList smsCenters = new SmsCentersList(SmeProperties.Templates.SMSCENTERS_XML, SmeProperties.Templates.ROUTES_XML);

      // Start SMS centers reloader
      new SMSCentersReloader(smsCenters).start();

      ConnectionPoolFactory.init("config");

      final SMPPMultiplexor multiplexor = new SMPPMultiplexor(new PropertiesConfig("config/network.properties"));
      multiplexor.connect();

      final SubscriptionStateMachine stateMachine = new SubscriptionStateMachine(multiplexor.getOutQueue(), smsCenters);

      new SMPPWorker(multiplexor, stateMachine).start();

    } catch (Throwable e) {
      e.printStackTrace();
    }
  }

  private static class SMSCentersReloader extends IterativeWorker {
    private final SmsCentersList smsCenters;

    SMSCentersReloader(SmsCentersList smsCenters) {
      super(null, SmeProperties.Templates.RELOAD_INTERVAL);
      this.smsCenters = smsCenters;
    }

    public synchronized void iterativeWork() {
      try {
        smsCenters.reload();
      } catch (SmsCentersListException e) {
        e.printStackTrace();
      }
    }

    protected void stopCurrentWork() {
    }
  }
}
