package ru.novosoft.smsc.util.config;

/**
 * User: artem
 * Date: 16.02.2007
 */

public final class ConfigException extends Exception{
  public ConfigException() {
  }

  public ConfigException(String message) {
    super(message);
  }

  public ConfigException(Throwable cause) {
    super(cause);
  }

  public ConfigException(String message, Throwable cause) {
    super(message, cause);
  }
}
