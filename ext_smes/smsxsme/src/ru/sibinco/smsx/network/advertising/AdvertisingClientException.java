package ru.sibinco.smsx.network.advertising;

/**
 * User: artem
 * Date: 17.07.2007
 */

public class AdvertisingClientException extends Exception {
  AdvertisingClientException() {
  }

  AdvertisingClientException(String message) {
    super(message);
  }

  AdvertisingClientException(Throwable cause) {
    super(cause);
  }

  AdvertisingClientException(String message, Throwable cause) {
    super(message, cause);
  }
}
