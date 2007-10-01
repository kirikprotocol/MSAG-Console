package ru.sibinco.smsx.engine.service.sponsored.commands;

import ru.sibinco.smsx.engine.service.Command;

import java.util.Date;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SponsoredRegisterAbonentCmd extends Command {

  public static final int STATUS_INVALID_SUBSCRIPTION_COUNT = STATUS_INT + 1;
  public static final int STATUS_SUBSCRIPTION_LOCKED = STATUS_INT + 2;

  private String abonentAddress;
  private int count;

  private boolean abonentExists;
  private Date profileChangeDate;

  public boolean isAbonentExists() {
    return abonentExists;
  }

  public void setAbonentExists(boolean abonentExists) {
    this.abonentExists = abonentExists;
  }

  public Date getProfileChangeDate() {
    return profileChangeDate;
  }

  public void setProfileChangeDate(Date profileChangeDate) {
    this.profileChangeDate = profileChangeDate;
  }

  public String getAbonentAddress() {
    return abonentAddress;
  }

  public void setAbonentAddress(String abonentAddress) {
    this.abonentAddress = abonentAddress;
  }

  public int getCount() {
    return count;
  }

  public void setCount(int count) {
    this.count = count;
  }

  public interface Receiver {
    public void execute(SponsoredRegisterAbonentCmd cmd);
  }
}
