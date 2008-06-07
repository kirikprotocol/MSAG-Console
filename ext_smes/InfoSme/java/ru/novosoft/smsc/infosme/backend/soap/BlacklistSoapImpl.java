package ru.novosoft.smsc.infosme.backend.soap;

import org.apache.log4j.Category;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.jsp.SMSCAppContextImpl;

public class BlacklistSoapImpl implements BlacklistSoap {
  private static final Category logger = Category.getInstance(BlacklistSoapImpl.class);

  public boolean add(String msisdn) throws java.rmi.RemoteException {
    try {
      InfoSmeContext.getInstance(SMSCAppContextImpl.getInstance(), "InfoSme").getBlackListManager().add(msisdn);
      return true;
    } catch (Exception e) {
      logger.error(e,e);
      return false;
    }
  }

  public boolean remove(String msisdn) throws java.rmi.RemoteException {
    try {
      InfoSmeContext.getInstance(SMSCAppContextImpl.getInstance(), "InfoSme").getBlackListManager().remove(msisdn);
      return true;
    } catch (Exception e) {
      logger.error(e,e);
      return false;
    }
  }

}
