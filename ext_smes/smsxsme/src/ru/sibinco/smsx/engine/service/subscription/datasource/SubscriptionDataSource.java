package ru.sibinco.smsx.engine.service.subscription.datasource;

import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 28.07.2008
 */

public interface SubscriptionDataSource {

  public boolean checkSubscription(String address) throws DataSourceException;
  public void shutdown();
}
