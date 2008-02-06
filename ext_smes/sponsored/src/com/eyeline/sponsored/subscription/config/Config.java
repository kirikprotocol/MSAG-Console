package com.eyeline.sponsored.subscription.config;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.xml.XmlConfigSection;

/**
 * User: artem
 * Date: 05.02.2008
 */

public class Config extends com.eyeline.sponsored.config.Config {

  private String subscriptionSql;

  public Config(XmlConfig config) throws ConfigException {
    super(config);

    final XmlConfigSection subscr = config.getSection("subscription");
    if (subscr == null)
      throw new ConfigException("Subsctintion section not found");

    final XmlConfigSection storage = subscr.getSection("storage");
    if (storage == null)
      throw new ConfigException("Storage section not found inside subscription");

    subscriptionSql = storage.getString("subscriptionSql");
  }

  public String getSubscriptionSql() {
    return subscriptionSql;
  }
}
