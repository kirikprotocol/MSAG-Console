package ru.sibinco.smsx.engine.service.subscription;

import ru.sibinco.smsx.engine.service.subscription.commands.SubscriptionCheckCmd;
import ru.sibinco.smsx.engine.service.subscription.datasource.SubscriptionDataSource;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.utils.DataSourceException;
import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 28.07.2008
 */

class SubscriptionProcessor implements SubscriptionCheckCmd.Receiver {

  private static final Category log = Category.getInstance(SubscriptionProcessor.class);

  private final SubscriptionDataSource ds;

  public SubscriptionProcessor(SubscriptionDataSource ds) {
    this.ds = ds;
  }

  public boolean execute(SubscriptionCheckCmd cmd) throws CommandExecutionException {
    if (cmd.getAddress() == null || cmd.getAddress().trim().length() == 0)
      throw new CommandExecutionException("Invalid address", SubscriptionCheckCmd.ERR_INV_MSISDN);

    try {
      return ds.checkSubscription(cmd.getAddress());
    } catch (DataSourceException e) {
      log.error(e, e);
      throw new CommandExecutionException(e.getMessage(), SubscriptionCheckCmd.ERR_SYS_ERROR);
    }
  }
}
