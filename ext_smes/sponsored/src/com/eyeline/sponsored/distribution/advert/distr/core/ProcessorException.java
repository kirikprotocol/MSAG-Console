package com.eyeline.sponsored.distribution.advert.distr.core;

/**
 * User: artem
 * Date: 02.02.2008
 */

public class ProcessorException extends Exception {
  ProcessorException() {
  }

  ProcessorException(String message) {
    super(message);
  }

  ProcessorException(String message, Throwable cause) {
    super(message, cause);
  }

  ProcessorException(Throwable cause) {
    super(cause);
  }
}
