package com.eyeline.sponsored.distribution.advert.distr.adv;

/**
 * User: artem
 * Date: 22.04.2008
 */

public class AdvertisingClientFactory {
  private final String host;
  private final int port;
  private final int connectionTimeout;

  public AdvertisingClientFactory(String host, int port, int connectionTimeout) {
    this.host = host;
    this.port = port;
    this.connectionTimeout = connectionTimeout;
  }

  public AdvertisingClient createClient() {
    return new AdvertisingClient(host, port, connectionTimeout);
  }
}
