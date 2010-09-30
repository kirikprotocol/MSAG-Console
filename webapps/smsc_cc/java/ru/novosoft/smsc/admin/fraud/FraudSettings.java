package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public class FraudSettings implements Serializable {

  private static transient final ValidationHelper vh = new ValidationHelper(FraudSettings.class);

  private int tail;
  private boolean enableCheck;
  private boolean enableReject;
  private Collection<Address> whiteList = new ArrayList<Address>();

  FraudSettings() {}

  FraudSettings(FraudSettings copy) {
    this.tail = copy.tail;
    this.enableCheck = copy.enableCheck;
    this.enableReject = copy.enableReject;
    this.whiteList = new ArrayList<Address>(copy.whiteList);
  }

  public int getTail() {
    return tail;
  }

  public void setTail(int tail) throws AdminException {
    vh.checkPositive("tail", tail);
    this.tail = tail;
  }

  public boolean isEnableCheck() {
    return enableCheck;
  }

  public void setEnableCheck(boolean enableCheck) {
    this.enableCheck = enableCheck;
  }

  public boolean isEnableReject() {
    return enableReject;
  }

  public void setEnableReject(boolean enableReject) {
    this.enableReject = enableReject;
  }

  public Collection<Address> getWhiteList() {
    return whiteList;
  }

  public void setWhiteList(Collection<Address> whiteList) throws AdminException {
    vh.checkNoNulls("whiteList", whiteList);
    this.whiteList = whiteList;
  }

  public FraudSettings cloneSettings() {
    return new FraudSettings(this);
  }
}
