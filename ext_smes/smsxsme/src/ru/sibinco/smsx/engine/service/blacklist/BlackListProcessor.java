package ru.sibinco.smsx.engine.service.blacklist;

import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Address;
import ru.aurorisoft.smpp.SMPPAddressException;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListAddCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListCheckMsisdnCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListRemoveCmd;
import ru.sibinco.smsx.engine.service.blacklist.datasource.BlackListDataSource;

/**
 * User: artem
 * Date: 10.07.2007
 */

class BlackListProcessor implements BlackListAddCmd.Receiver, BlackListRemoveCmd.Receiver, BlackListCheckMsisdnCmd.Receiver {

  private static final Category log = Category.getInstance("BLACKLIST");

  private final BlackListDataSource ds;

  BlackListProcessor(BlackListDataSource ds) {
    this.ds = ds;
  }

  private static String prepareMsisdn(String msisdn) throws SMPPAddressException {
    final Address addr = new Address(msisdn);
    final StringBuilder buffer = new StringBuilder(msisdn.length()+10);
    buffer.append('.').append(addr.getTon()).append('.').append(addr.getNpi()).append('.').append(addr.getAddress());
    return buffer.toString();
  }

  public void execute(BlackListAddCmd cmd) {
    if (cmd == null)
      return;

    String msisdn = cmd.getMsisdn();

    try {
      if (log.isInfoEnabled())
        log.info("Add msisdn=" + msisdn);
      msisdn = prepareMsisdn(msisdn);

      if (!ds.isMsisdnInBlackList(msisdn))
        ds.addMsisdnToBlackList(msisdn);

      cmd.update(BlackListAddCmd.STATUS_SUCCESS);

    } catch (Throwable e) {
      log.error("Can't add msisdn " + msisdn + " into black list",e);
      cmd.update(BlackListAddCmd.STATUS_SYSTEM_ERROR);
    }
  }

  public void execute(BlackListRemoveCmd cmd) {
    if (cmd == null)
      return;

    String msisdn = cmd.getMsisdn();

    try {
      if (log.isInfoEnabled())
        log.info("Remove msisdn=" + msisdn);
      msisdn = prepareMsisdn(msisdn);

      ds.removeMsisdnFromBlackList(msisdn);

      cmd.update(BlackListRemoveCmd.STATUS_SUCCESS);

    } catch (Throwable e) {
      log.error("Can't remove msisdn " + msisdn + " into black list",e);
      cmd.update(BlackListRemoveCmd.STATUS_SYSTEM_ERROR);
    }
  }

  public void execute(BlackListCheckMsisdnCmd cmd) {
    if (cmd == null)
      return;

    String msisdn = cmd.getMsisdn();

    try {
      if (log.isInfoEnabled())
        log.info("Chk msisdn=" + msisdn);
      msisdn = prepareMsisdn(msisdn);

      cmd.setInBlackList(ds.isMsisdnInBlackList(msisdn));

      cmd.update(BlackListRemoveCmd.STATUS_SUCCESS);

    } catch (Throwable e) {
      log.error("Can't check msisdn " + msisdn,e);
      cmd.update(BlackListRemoveCmd.STATUS_SYSTEM_ERROR);
    }
  }
}
