package ru.sibinco.smpp.ub_sme.inman;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class PDUException extends Exception {
  public PDUException(String message) {
    super(message);
  }

  public PDUException(String message, Throwable cause) {
    super(message, cause);
  }

}
