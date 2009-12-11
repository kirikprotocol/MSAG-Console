/**
 * BlacklistSoapHandler.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.blacklist;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.*;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListAddCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListRemoveCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListCheckMsisdnCmd;

class BlacklistSoapHandler implements BlacklistSoap {

  private static final Category log = Category.getInstance(BlacklistSoapHandler.class);

  BlacklistSoapHandler() {
  }

  public boolean add(String msisdn) throws java.rmi.RemoteException {
    final long start = System.currentTimeMillis();
    if (log.isInfoEnabled())
      log.info("Add to black list: msisdn=" + msisdn);
    try {                                                                 
      final BlackListAddCmd cmd = new BlackListAddCmd();
      cmd.setMsisdn(msisdn);

      Services.getInstance().getBlackListService().execute(cmd);

      return true;
    } catch (Throwable e) {
      log.error(e,e);
      return false;
    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public boolean remove(String msisdn) throws java.rmi.RemoteException {
    final long start = System.currentTimeMillis();
    if (log.isInfoEnabled())
      log.info("Remove from black list: msisdn=" + msisdn);

    try {
      final BlackListRemoveCmd cmd = new BlackListRemoveCmd();
      cmd.setMsisdn(msisdn);
      Services.getInstance().getBlackListService().execute(cmd);

      return true;

    } catch (Throwable e) {
      log.error(e,e);
      return false;
    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public boolean check(String msisdn) throws java.rmi.RemoteException {
    final long start = System.currentTimeMillis();
    if (log.isInfoEnabled())
      log.info("Check: msisdn=" + msisdn);

    try {
      final BlackListCheckMsisdnCmd cmd = new BlackListCheckMsisdnCmd();
      cmd.setMsisdn(msisdn);
      return Services.getInstance().getBlackListService().execute(cmd);

    } catch (Throwable e) {
      log.error(e,e);
      return false;
    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }
}
