package ru.sibinco.smsx.services.secret;

import ru.aurorisoft.smpp.Message;

/**
 * User: artem
 * Date: 25.01.2007
 */

final class ParsedMessage {
  private final Message message;
  private final SecretRequestParser.ParseResult parseResult;

  public ParsedMessage(Message message, SecretRequestParser.ParseResult parseResult) {
    this.message = message;
    this.parseResult = parseResult;
  }

  public Message getMessage() {
    return message;
  }

  public SecretRequestParser.ParseResult getParseResult() {
    return parseResult;
  }
}
