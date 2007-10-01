package ru.sibinco.smsx.network.advertising;

/**
 * User: artem
 * Date: 17.07.2007
 */

public interface AdvertisingClient {

  /**
   * Connect to advertising server
   */
  public void connect();

  /**
   * Close connection with advertising server
   */
  public void close();

  /**
   * Check connection with advertising server
   * @return connected
   */
  public boolean isConnected();

  /**
   * Get banner from server by service name and abonent address
   * @param serviceName name of service in advertising server
   * @param abonentAddress abonent address
   * @return banner
   * @throws AdvertisingClientException
   */
  public String getBanner(String serviceName, String abonentAddress) throws AdvertisingClientException;

  /**
   * Get banner from advertising server
   * @param serviceName serviceName name of service in advertising server
   * @param abonentAddress abonentAddress abonent address
   * @param transportType
   * @param maxBannerLength
   * @param charSet
   * @param clientId
   * @param transactionId
   * @return
   * @throws AdvertisingClientException
   */
  public String getBanner(String serviceName, String abonentAddress, int transportType, int maxBannerLength, int charSet, int clientId, int transactionId) throws AdvertisingClientException;

  /**
   * Get banner from advertising server
   * @param serviceName serviceName name of service in advertising server
   * @param abonentAddress abonentAddress abonent address
   * @param maxBannerLength max banner length
   * @return
   * @throws AdvertisingClientException
   */
  public String getBanner(String serviceName, String abonentAddress, int maxBannerLength) throws AdvertisingClientException;
}
