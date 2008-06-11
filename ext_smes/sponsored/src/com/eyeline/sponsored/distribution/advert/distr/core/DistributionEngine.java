package com.eyeline.sponsored.distribution.advert.distr.core;

import com.eyeline.sponsored.distribution.advert.config.DistributionInfo;

import javax.management.DynamicMBean;

/**
 * User: artem
 * Date: 22.02.2008
 */

public interface DistributionEngine {
  public void start();

  public void stop();

  public void addDistribution(DistributionInfo info);

  public DynamicMBean getMBean();
}
