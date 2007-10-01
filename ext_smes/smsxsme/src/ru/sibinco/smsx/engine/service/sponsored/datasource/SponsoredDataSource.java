package ru.sibinco.smsx.engine.service.sponsored.datasource;

import com.eyeline.sme.utils.ds.DataSourceException;

import java.util.Collection;
import java.util.Date;

/**
 * User: artem
 * Date: 02.07.2007
 */

public interface SponsoredDataSource {

  public Collection getAbonents(int subscriptionCount, int start, int portion) throws DataSourceException;
  public int getTotalAbonentsCount(int subscriptionCount) throws DataSourceException;
  public int getTotalMessagesCount(int subscriptionCount) throws DataSourceException;
  public void decreaseMessagesCount(String abonent) throws DataSourceException;

  public DistributionInfo getDistributionInfoByAbonent(String abonent) throws DataSourceException;
  public void saveDistributionInfo(DistributionInfo info) throws DataSourceException;

  public void updateSponsoredStatus(String address) throws DataSourceException;

  public void release();
}
