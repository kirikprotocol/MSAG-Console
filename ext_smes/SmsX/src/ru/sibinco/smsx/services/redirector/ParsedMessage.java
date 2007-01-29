package ru.sibinco.smsx.services.redirector;

import ru.aurorisoft.smpp.Message;

/**
 * User: artem
 * Date: 26.01.2007
 */
final class ParsedMessage {
  private final Message message;
  private final String toAddress;

  public ParsedMessage(Message message, String toAddress) {
    this.message = message;
    this.toAddress = toAddress;
  }

  public Message getMessage() {
    return message;
  }

  public String getToAddress() {
    return toAddress;
  }
}
