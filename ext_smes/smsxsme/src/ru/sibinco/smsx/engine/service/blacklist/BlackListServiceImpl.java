package ru.sibinco.smsx.engine.service.blacklist;

import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.blacklist.datasource.BlackListDataSource;
import ru.sibinco.smsx.engine.service.blacklist.datasource.DBBlackListDataSource;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListAddCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListRemoveCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListCheckMsisdnCmd;

import java.io.File;

import com.eyeline.utils.config.xml.XmlConfig;

/**
 * User: artem
 * Date: 10.07.2007
 */

public class BlackListServiceImpl implements BlackListService {

  private final BlackListDataSource ds;
  private final BlackListProcessor processor;

  public BlackListServiceImpl(XmlConfig config) {
    try {
      ds = new DBBlackListDataSource();
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

  public Object getMBean(String domain) {
    return null;
  }

  public void execute(BlackListAddCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public void execute(BlackListRemoveCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public boolean execute(BlackListCheckMsisdnCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }
}
