package com.eyeline.sponsored.distribution.advert.distr.adv;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class AdvertisingException extends Exception {
  AdvertisingException() {
  }

  AdvertisingException(String message) {
    super(message);
  }

  AdvertisingException(String message, Throwable cause) {
    super(message, cause);
  }

  AdvertisingException(Throwable cause) {
    super(cause);
  }
}
