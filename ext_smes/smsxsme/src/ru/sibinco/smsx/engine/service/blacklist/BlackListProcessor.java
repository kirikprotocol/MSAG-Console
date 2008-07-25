package ru.sibinco.smsx.engine.service.blacklist;

import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Address;
import ru.aurorisoft.smpp.SMPPAddressException;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListAddCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListCheckMsisdnCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListRemoveCmd;
import ru.sibinco.smsx.engine.service.blacklist.datasource.BlackListDataSource;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

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

  public void execute(BlackListAddCmd cmd) throws CommandExecutionException {
    String msisdn = cmd.getMsisdn();

    try {
      if (log.isInfoEnabled())
        log.info("Add msisdn=" + msisdn);
      msisdn = prepareMsisdn(msisdn);

      if (!ds.isMsisdnInBlackList(msisdn))
        ds.addMsisdnToBlackList(msisdn);

    } catch (Throwable e) {
      log.error("Can't add msisdn " + msisdn + " into black list",e);
      throw new CommandExecutionException("Error: " + e.getMessage(), BlackListAddCmd.ERR_SYS_ERROR);
    }
  }

  public void execute(BlackListRemoveCmd cmd) throws CommandExecutionException {
    String msisdn = cmd.getMsisdn();

    try {
      if (log.isInfoEnabled())
        log.info("Remove msisdn=" + msisdn);
      msisdn = prepareMsisdn(msisdn);

      ds.removeMsisdnFromBlackList(msisdn);

    } catch (Throwable e) {
      log.error("Can't remove msisdn " + msisdn + " into black list",e);
      throw new CommandExecutionException("Error: " + e.getMessage(), BlackListCheckMsisdnCmd.ERR_SYS_ERROR);
    }
  }

  public boolean execute(BlackListCheckMsisdnCmd cmd) throws CommandExecutionException {
    String msisdn = cmd.getMsisdn();

    try {
      if (log.isInfoEnabled())
        log.info("Chk msisdn=" + msisdn);
      msisdn = prepareMsisdn(msisdn);

      return ds.isMsisdnInBlackList(msisdn);

    } catch (Throwable e) {
      log.error("Can't check msisdn " + msisdn,e);
      throw new CommandExecutionException("Error: " + e.getMessage(), BlackListCheckMsisdnCmd.ERR_SYS_ERROR);
    }
  }
}
