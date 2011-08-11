package mobi.eyeline.informer.admin.siebel;

/**
* author: Aleksandr Khalitov
*/
public class DeliveryMessageState {
  private final State state;
  private final String smppCode;
  private final String smppCodeDescription;

  public DeliveryMessageState(State state, String smppCode, String smppCodeDescription) {
    this.state = state;
    this.smppCode = smppCode;
    this.smppCodeDescription = smppCodeDescription;
  }

  public State getState() {
    return state;
  }

  public String getSmppCode() {
    return smppCode;
  }

  public String getSmppCodeDescription() {
    return smppCodeDescription;
  }

  @Override
  public String toString() {
    final StringBuilder sb = new StringBuilder();
    sb.append("DeliveryState");
    sb.append("{state=").append(state);
    sb.append(", smppCode='").append(smppCode).append('\'');
    sb.append(", smppCodeDescription='").append(smppCodeDescription).append('\'');
    sb.append('}');
    return sb.toString();
  }

  public static enum State {
    ENROUTE,
    DELIVERED,
    EXPIRED,
    DELETED,
    UNDELIVERABLE,
    ACCEPTED,
    UNKNOWN,
    REJECTED,
    ERROR
  }
}
