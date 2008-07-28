package ru.sibinco.smsx.engine.soaphandler.smsxsubscription;

import ru.sibinco.smsx.engine.service.subscription.commands.SubscriptionCheckCmd;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import org.apache.log4j.Category;

class SmsXSubscriptionSoapHandler implements SmsXSubscription{

  private static final Category log = Category.getInstance(SmsXSubscription.class);

  private static final int STATUS_OK = 0;
  private static final int STATUS_SYS_ERROR = -1;
  private static final int STATUS_INVALID_MSISDN = -2;

  public SmsXSubscriptionSoapHandler(String configDir) {
  }

  public CheckSubscriptionResp checkSubscription(String msisdn) throws java.rmi.RemoteException {
    if (log.isDebugEnabled())
      log.debug("Subscr check req: msisdn=" + msisdn);

    CheckSubscriptionResp resp = new CheckSubscriptionResp();

    try {
      SubscriptionCheckCmd cmd = new SubscriptionCheckCmd();
      cmd.setAddress(msisdn);

      boolean res = Services.getInstance().getSubscriptionService().execute(cmd);
      resp.setStatus(STATUS_OK);
      resp.setSubscribed(res);
    } catch (CommandExecutionException e) {
      log.error("Subscr check failed.", e);
      if (e.getErrCode() == SubscriptionCheckCmd.ERR_INV_MSISDN)
        resp.setStatus(STATUS_INVALID_MSISDN);
      else
        resp.setStatus(STATUS_SYS_ERROR);
    }
    return resp;
  }
}
