package ru.sibinco.smsx.network.advertising;

import com.lorissoft.advertising.util.Encode;
import ru.aurorisoft.smpp.Address;

/**
 * User: artem
 * Date: 17.07.2007
 */

class AdvertisingClientImpl implements AdvertisingClient {

  private final com.lorissoft.advertising.syncclient.AdvertisingClientImpl impl;

  AdvertisingClientImpl(String host, int port, long timeout) {
    impl = new com.lorissoft.advertising.syncclient.AdvertisingClientImpl();
    impl.setIp(host);
    impl.setPort(port);
    impl.setTimeout(timeout);
  }

  public void connect() {
    impl.init(null);
  }

  public void close() {
    impl.close();
  }

  public boolean isConnected() {
    return impl.isConnected();
  }

  public String getBanner(String serviceName, String abonentAddress) throws AdvertisingClientException {
    return getBanner(serviceName, abonentAddress, 1, 0, 1, 1, 0);
  }

  public String getBanner(String serviceName, String abonentAddress, int maxBannerLength) throws AdvertisingClientException {
    return getBanner(serviceName, abonentAddress, 1, maxBannerLength, 1, 1, 0);
  }

  public String getBanner(String serviceName, String abonentAddress, int transportType, int maxBannerLength, int charSet, int clientId, int transactionId) throws AdvertisingClientException {
    try {
      final Address address = new Address(abonentAddress);
      final String abonent = "." + address.getTon() + "." + address.getNpi() + "." + address.getAddress();

      final byte[] banner = impl.getLikelyBanner(abonent.getBytes("ISO8859-1"), abonent.length(),
                                                 serviceName.getBytes("ISO8859-1"),
                                                 transportType, maxBannerLength, charSet, clientId, transactionId);

      return banner == null ? null : Encode.decodeUTF16(banner);

    } catch (Throwable e) {
      throw new AdvertisingClientException(e);
    }
  }
}
