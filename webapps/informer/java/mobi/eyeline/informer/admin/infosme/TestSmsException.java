package mobi.eyeline.informer.admin.infosme;

/**
 * User: artem
 * Date: 15.03.11
 */
public class TestSmsException extends InfosmeException {

  private int smppStatus;

  TestSmsException(int smppStatus) {
    super("interaction_error");
    this.smppStatus = smppStatus;
  }

  public int getSmppStatus() {
    return smppStatus;
  }
}
