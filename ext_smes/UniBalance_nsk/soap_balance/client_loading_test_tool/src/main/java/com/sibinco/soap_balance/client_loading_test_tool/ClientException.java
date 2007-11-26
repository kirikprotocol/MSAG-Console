package com.sibinco.soap_balance.client_loading_test_tool;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class ClientException extends Exception {
  public static final int CONNECT_FAILED = 0;
  public static final int SERVER_RESPONSE_ERROR = 1;
  public static final int POOL_EXCEPTION = 2;
  public static final int UNKNOWN = 3;

  private final static String messages[] = {
      "Can't connect to server",
      "Server response error",
      "Pool exception",
      "Unknown exception"
  };

  private int type;

  public ClientException(int type) {
    super(ClientException.getMessage(type));
    this.type = type;
  }

  public ClientException(int type, Throwable cause) {
    super(ClientException.getMessage(type), cause);
    this.type = type;
  }

  public ClientException(int type, String message) {
    super(message);
    this.type = type;
  }

  public ClientException(int type, String message, Throwable cause) {
    super(message, cause);
    this.type = type;
  }

  public ClientException(String message) {
    super(message);
    this.type = -1;
  }

  public ClientException(String message, Throwable cause) {
    super(message, cause);
    this.type = -1;
  }

  public int getType() {
    return type;
  }

  private static String getMessage(int type) {
    if (type < 0 || type < ClientException.UNKNOWN) {
      return ClientException.messages[type];
    }
    return ClientException.messages[ClientException.UNKNOWN];
  }

}
