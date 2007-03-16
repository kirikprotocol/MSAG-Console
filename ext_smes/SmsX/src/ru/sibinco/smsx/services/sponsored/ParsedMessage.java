package ru.sibinco.smsx.services.sponsored;

import ru.aurorisoft.smpp.Message;

/**
 * User: artem
 * Date: 13.03.2007
 */

final class ParsedMessage {
  public static final int SUBSCRIBE = 0;
  public static final int UNSUBSCRIBE = 1;

  private final int type;
  private final int count;
  private final Message sourceMessage;

  public ParsedMessage(Message sourceMessage, int type, int count) {
    this.type = type;
    this.count = count;
    this.sourceMessage = sourceMessage;
  }

  public int getType() {
    return type;
  }

  public int getCount() {
    return count;
  }

  public Message getSourceMessage() {
    return sourceMessage;
  }
}
