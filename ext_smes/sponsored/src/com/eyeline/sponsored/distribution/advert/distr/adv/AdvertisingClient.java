package com.eyeline.sponsored.distribution.advert.distr.adv;

import com.lorissoft.advertising.util.Encode;
import com.lorissoft.advertising.commons.RequestResult;
import ru.aurorisoft.smpp.Address;

/**
 * User: artem
 * Date: 17.07.2007
 */

public class AdvertisingClient {

  private final com.lorissoft.advertising.syncclient.AdvertisingClientImpl impl;

  public AdvertisingClient(String host, int port, long timeout) {
    impl = new com.lorissoft.advertising.syncclient.AdvertisingClientImpl();
    impl.setIp(host);
    impl.setPort(port);
    impl.setTimeout(timeout);
  }

  public void connect() {
    impl.init(null);
    impl.connect();
  }

  public void close() {
    impl.close();
  }

  public boolean isConnected() {
    return impl.isConnected();
  }

  public BannerWithInfo getBannerWithInfo(String serviceName, String abonentAddress) throws AdvertisingException {
    return getBannerWithInfo(serviceName, abonentAddress, 1, 0, 1, 1, 0);
  }

  public BannerWithInfo getBannerWithInfo(String serviceName, String abonentAddress, int maxBannerLength) throws AdvertisingException {
    return getBannerWithInfo(serviceName, abonentAddress, 1, maxBannerLength, 1, 1, 0);
  }

  public BannerWithInfo getBannerWithInfo(String serviceName, String abonentAddress, int transportType, int maxBannerLength, int charSet, int clientId, int transactionId) throws AdvertisingException {
    try {
      final Address address = new Address(abonentAddress);
      final String abonent = "." + address.getTon() + '.' + address.getNpi() + '.' + address.getAddress();

      RequestResult banner = impl.getLikelyBannerWithId(abonent.getBytes("ISO8859-1"), abonent.length(),
                                                 serviceName.getBytes("ISO8859-1"),
                                                 transportType, maxBannerLength, charSet, clientId, transactionId);

      if (banner == null || banner.getBannerText() == null || banner.getBannerText().length == 0)
        return null;

      return new BannerWithInfo(Encode.decodeUTF16(banner.getBannerText()), banner.getAdvertiserId());
    } catch (Throwable e) {
      throw new AdvertisingException(e);
    }
  }

  public String getBanner(String serviceName, String abonentAddress) throws AdvertisingException {
    return getBanner(serviceName, abonentAddress, 1, 0, 1, 1, 0);
  }

  public String getBanner(String serviceName, String abonentAddress, int maxBannerLength) throws AdvertisingException {
    return getBanner(serviceName, abonentAddress, 1, maxBannerLength, 1, 1, 0);
  }

  public String getBanner(String serviceName, String abonentAddress, int transportType, int maxBannerLength, int charSet, int clientId, int transactionId) throws AdvertisingException {
    try {
      final Address address = new Address(abonentAddress);
      final String abonent = "." + address.getTon() + '.' + address.getNpi() + '.' + address.getAddress();

      final byte[] banner = impl.getLikelyBanner(abonent.getBytes("ISO8859-1"), abonent.length(),
                                                 serviceName.getBytes("ISO8859-1"),
                                                 transportType, maxBannerLength, charSet, clientId, transactionId);

      return banner == null ? null : Encode.decodeUTF16(banner);

    } catch (Throwable e) {
      throw new AdvertisingException(e);
    }
  }
}
