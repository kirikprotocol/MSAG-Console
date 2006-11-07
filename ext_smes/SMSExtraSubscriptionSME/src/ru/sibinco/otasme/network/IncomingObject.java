package ru.sibinco.otasme.network;

import ru.aurorisoft.smpp.Message;

public class IncomingObject {
  private Message message = null;

  public IncomingObject(Message message) {
    this.message = message;
  }

  public Message getMessage() {
    return message;
  }
}
