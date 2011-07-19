package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.delivery.MessageState;

/**
 * @author Artem Snopkov
 */
public enum MsgState {
  New(MessageState.New, MessageState.Process),
  Retry(MessageState.Retry),
  Sent(MessageState.Sent),
  Delivered(MessageState.Delivered),
  Failed(MessageState.Failed),
  Expired(MessageState.Expired),
  Finalized(MessageState.Delivered, MessageState.Failed, MessageState.Expired);

  private MessageState[] states;

  MsgState(MessageState... states) {
    this.states = states;
  }

  public MessageState[] toMessageStates() {
    return states;
  }

}
