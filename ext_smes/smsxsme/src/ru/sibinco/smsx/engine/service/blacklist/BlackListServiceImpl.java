package ru.sibinco.smsx.engine.service.blacklist;

import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.blacklist.datasource.BlackListDataSource;
import ru.sibinco.smsx.engine.service.blacklist.datasource.DBBlackListDataSource;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListAddCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListRemoveCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListCheckMsisdnCmd;

import java.io.File;

/**
 * User: artem
 * Date: 10.07.2007
 */

class BlackListServiceImpl implements BlackListService {

  private final BlackListDataSource ds;
  private final BlackListProcessor processor;

  BlackListServiceImpl(String configDir) {
    try {
      ds = new DBBlackListDataSource(new File(configDir, "services/blacklist/blacklist.sql").getAbsolutePath(), "");
//      ds = new PersonalizationBlackListDataSource();
      processor = new BlackListProcessor(ds);

    } catch (Throwable e) {
      throw new ServiceInitializationException(e);
    }
  }

  public void startService() {
  }

  public void stopService() {
    ds.release();
  }

  public void execute(BlackListAddCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(BlackListRemoveCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(BlackListCheckMsisdnCmd cmd) {
    processor.execute(cmd);
  }
}
