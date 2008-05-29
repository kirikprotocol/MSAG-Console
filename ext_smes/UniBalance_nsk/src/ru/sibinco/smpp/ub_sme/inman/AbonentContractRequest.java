package ru.sibinco.smpp.ub_sme.inman;

import ru.sibinco.smpp.ub_sme.util.VarString;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 06.12.2006
 * Time: 18:44:31
 * To change this template use File | Settings | File Templates.
 */
public class AbonentContractRequest extends InManPDU {

  boolean useCache;
  VarString subscriberNumber;

  public final static byte TAG = 0x06;

  public AbonentContractRequest() {
    super(TAG);
  }

  public void setUseCache(boolean useCache) {
    this.useCache = useCache;
  }

  public void setSubscriberNumber(String subscriberNumber) {
    this.subscriberNumber = new VarString(subscriberNumber);
  }

  protected void fillData() {
    if (subscriberNumber == null) {
      subscriberNumber = new VarString("");
    }
    int length = HEADER_LENGTH + 1 + subscriberNumber.getLength();
    data = new byte[length];
    super.fillData();
    int p = HEADER_LENGTH;
    data[p++] = (byte) (useCache ? 1 : 0);
    subscriberNumber.toBytesArray(data, p);
  }

  public String toString() {
    StringBuffer sb = new StringBuffer("AbonentContractRequest: sn=");
    sb.append(dialogID);
    sb.append(", useCache=");
    sb.append(useCache);
    sb.append(", subscriberNumber=");
    sb.append(subscriberNumber);
    return sb.toString();
  }

}
