package ru.sibinco.smsx.engine.service.sponsored.commands;

import ru.sibinco.smsx.engine.service.Command;

import java.util.Date;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SponsoredUnregisterAbonentCmd extends Command {
  public static final int STATUS_ABONENT_NOT_REGISTERED = STATUS_INT + 1;

  private String abonentAddress;

  private Date profileChangeDate;

  public String getAbonentAddress() {
    return abonentAddress;
  }

  public void setAbonentAddress(String abonentAddress) {
    this.abonentAddress = abonentAddress;
  }

  public Date getProfileChangeDate() {
    return profileChangeDate;
  }

  public void setProfileChangeDate(Date profileChangeDate) {
    this.profileChangeDate = profileChangeDate;
  }

  public interface Receiver {
    public void execute(SponsoredUnregisterAbonentCmd cmd);
  }
}
