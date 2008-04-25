package com.eyeline.sponsored.distribution.advert.config;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.xml.XmlConfigSection;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

/**
 * User: artem
 * Date: 05.02.2008
 */

public class Config extends com.eyeline.sponsored.config.Config {

  private int deliveriesFetchInterval;
  private int deliveriesSendSpeedLimit;
  private int deliveriesPrepareInterval;
  private int poolSize;
  private String engineType;

  private String deliveryStatsDataSource;
  private String deliveriesDataSource;

  private String storageSubscriptionSql;
  private String storageDistributionSql;

  private List<DistributionInfo> distrInfos;

  public Config(XmlConfig config) throws ConfigException {
    super(config);

    final XmlConfigSection distr = config.getSection("advert_distribution");
    if (distr == null)
      throw new ConfigException("advert_distribution section not found");

    final XmlConfigSection storage = distr.getSection("storage");
    if (storage == null)
      throw new ConfigException("storage section not found inside advert_distribution");

    deliveriesFetchInterval = distr.getInt("deliveriesFetchInterval");
    deliveriesSendSpeedLimit = distr.getInt("deliveriesSendSpeedLimit");
    deliveriesPrepareInterval = distr.getInt("deliveriesPrepareInterval");
    poolSize = distr.getInt("poolSize");
    engineType = distr.getString("engineType");

    deliveryStatsDataSource = distr.getString("deliveryStatsDataSource");
    deliveriesDataSource = distr.getString("deliveriesDataSource");

    storageSubscriptionSql = storage.getString("subscriptionSql");
    storageDistributionSql = storage.getString("distributionSql");

    distrInfos = new LinkedList<DistributionInfo>();

    final XmlConfigSection distrSection = distr.getSection("distributions");
    if (distrSection == null)
      throw new ConfigException("distributions section not found in advert_distribution");


    XmlConfigSection s;
    for (Iterator iter = distrSection.sections(); iter.hasNext();) {
      s = (XmlConfigSection)iter.next();
      final DistributionInfo info = new DistributionInfo();
      info.setDistributionName(s.getName());
      info.setAdvServiceName(s.getString("advServiceName"));
      info.setSrcAddress(s.getString("srcAddress"));
      try {
        info.setStartTime(parseTime(s.getString("startTime")));
      } catch (ParseException e) {
        throw new ConfigException("Invalid start time in distr " + s.getName() + ". Should be HH:mm:ss");
      }
      try {
        info.setEndTime(parseTime(s.getString("endTime")));
      } catch (ParseException e) {
        throw new ConfigException("Invalid end time in distr " + s.getName() + ". Should be HH:mm:ss");
      }
      distrInfos.add(info);
    }
  }

  private static Calendar parseTime(String time) throws ParseException {
    final SimpleDateFormat df = new SimpleDateFormat("HH:mm:ss");
    final Calendar res = Calendar.getInstance();
    res.setTime(df.parse(time));
    return res;
  }

  public String getStorageSubscriptionSql() {
    return storageSubscriptionSql;
  }

  public String getStorageDistributionSql() {
    return storageDistributionSql;
  }

  public List<DistributionInfo> getDistrInfos() {
    return distrInfos;
  }

  public int getDeliveriesFetchInterval() {
    return deliveriesFetchInterval;
  }

  public int getDeliveriesSendSpeedLimit() {
    return deliveriesSendSpeedLimit;
  }

  public int getDeliveriesPrepareInterval() {
    return deliveriesPrepareInterval;
  }

  public String getEngineType() {
    return engineType;
  }

  public String getDeliveryStatsDataSource() {
    return deliveryStatsDataSource;
  }

  public String getDeliveriesDataSource() {
    return deliveriesDataSource;
  }

  public int getPoolSize() {
    return poolSize;
  }
}
