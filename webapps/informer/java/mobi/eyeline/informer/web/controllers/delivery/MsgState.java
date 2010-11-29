package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.delivery.MessageState;

/**
 * @author Artem Snopkov
 */
public enum MsgState {

  Finalized(MessageState.Delivered, MessageState.Failed, MessageState.Expired),
  Indelivery(MessageState.New, MessageState.Process),
  New(MessageState.New),
  Process(MessageState.Process),
  Delivered(MessageState.Delivered),
  Failed(MessageState.Failed),
  Expired(MessageState.Expired);

  private MessageState[] states;

  MsgState(MessageState... states) {
    this.states = states;
  }

  public MessageState[] toMessageStates() {
    return states;
  }

}
