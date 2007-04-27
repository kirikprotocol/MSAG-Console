package ru.sibinco.smsx.services.redirector;

import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.services.redirector.redirects.Redirect;

/**
 * User: artem
 * Date: 26.01.2007
 */
final class ParsedMessage {
  private final Message message;
  private final Redirect redirect;

  public ParsedMessage(Message message, Redirect redirect) {
    this.message = message;
    this.redirect = redirect;
  }

  public Message getMessage() {
    return message;
  }

  public Redirect getRedirect() {
    return redirect;
  }
}
