package ru.sibinco.smsx.engine.service.secret.datasource;

import java.util.Collection;

/**
 * User: artem
 * Date: Aug 28, 2007
 */

public class SecretUserWithMessages {
  private final SecretUser user;
  private final Collection messages;

  public SecretUserWithMessages(SecretUser user, Collection messages) {
    this.user = user;
    this.messages = messages;
  }

  public SecretUser getUser() {
    return user;
  }

  public Collection getMessages() {
    return messages;
  }
}
