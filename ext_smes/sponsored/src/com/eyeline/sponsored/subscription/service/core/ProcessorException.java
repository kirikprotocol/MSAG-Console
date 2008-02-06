package com.eyeline.sponsored.subscription.service.core;

/**
 *
 * @author artem
 */
public class ProcessorException extends Exception {

  ProcessorException(String message, Throwable cause) {
    super(message, cause);    
  }

  ProcessorException(String message) {
    super(message);
  }
  
}
