package ru.sibinco.smsx.services.calendar;

import ru.aurorisoft.smpp.Message;

/**
 * User: artem
 * Date: 25.01.2007
 */
final class ParsedMessage {
  private final Message message;
  private final CalendarRequestParser.ParseResult parseResult;

  public ParsedMessage(Message message, CalendarRequestParser.ParseResult parseResult) {
    this.message = message;
    this.parseResult = parseResult;
  }

  public Message getMessage() {
    return message;
  }

  public CalendarRequestParser.ParseResult getParseResult() {
    return parseResult;
  }
}
