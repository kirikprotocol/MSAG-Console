package ru.sibinco.smsx.engine.service.subscription;

import ru.sibinco.smsx.engine.service.subscription.commands.SubscriptionCheckCmd;
import ru.sibinco.smsx.engine.service.subscription.datasource.SubscriptionDataSource;
import ru.sibinco.smsx.engine.service.subscription.datasource.DBSubscriptionDataSource;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import com.eyeline.utils.config.xml.XmlConfig;

/**
 * User: artem
 * Date: 28.07.2008
 */

public class SubscriptionServiceImpl implements SubscriptionService {

  private final SubscriptionDataSource ds;
  private final SubscriptionProcessor processor;

  public SubscriptionServiceImpl(XmlConfig config) {
    try {
      this.ds = new DBSubscriptionDataSource();
      this.processor = new SubscriptionProcessor(ds);
    } catch (Exception e) {
      throw new ServiceInitializationException(e);
    }
  }

  public void startService() {
  }

  public void stopService() {
    ds.shutdown();
  }

  public Object getMBean(String domain) {
    return null;
  }

  public boolean execute(SubscriptionCheckCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }
}
