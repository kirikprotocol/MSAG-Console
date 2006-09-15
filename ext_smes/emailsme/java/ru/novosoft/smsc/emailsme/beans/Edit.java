package ru.novosoft.smsc.emailsme.beans;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.emailsme.backend.EmailSmeMessages;
import ru.novosoft.smsc.emailsme.backend.EmailSmeService;
import ru.novosoft.smsc.emailsme.backend.Profile;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * User: igork
 * Date: 15.09.2003
 */

public class Edit extends SmeBean {

  public final static int RESULT_DELETE = PRIVATE_RESULT + 1;

  private Profile profile = new Profile("","",-1,"","");

  private String mbDone = null;
  private String mbDel = null;
  private String mbCancel = null;
  private boolean create = false;

  private EmailSmeService service;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    service = new EmailSmeService(appContext);

    if (!isInitialized() && create)
      profile = new Profile("","", 0, "", "");

    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbDone != null) return done();
    if (mbDel != null) return delete();
    if (mbCancel != null) return RESULT_DONE;
    return result;
  }

  private int delete() {
    return service.deleteProfile(profile) ? RESULT_DELETE : error(EmailSmeMessages.errors.cantDeleteProfile);
  }

  private int done() {
    // validate profile
    if (!Mask.isMaskValid(profile.getAddr()))
      return error(EmailSmeMessages.errors.invalidAddress);

    if (profile.getDayLimit() < 0)
      return error(EmailSmeMessages.errors.invalidDayLimit);

    if (create)
      return service.addProfile(profile) ? RESULT_DONE : error(EmailSmeMessages.errors.cantSaveProfile);
    else
      return service.updateProfile(profile) ? RESULT_DONE : error(EmailSmeMessages.errors.cantSaveProfile);
  }

  public String getAddr() {

    return profile.getAddr();
  }

  public void setAddr(String addr) {
    profile.setAddr(addr);
  }

  public String getUserid() {
    return profile.getUserid();
  }

  public void setUserid(String userid) {
    profile.setUserid(userid);
  }

  public int getDayLimitInt() {
    return profile.getDayLimit();
  }

  public void setDayLimitInt(int dayLimit) {
    profile.setDayLimit(dayLimit);
  }

  public String getDayLimit() {
    return String.valueOf(profile.getDayLimit());
  }

  public void setDayLimit(String dayLimit) {
    try {
      profile.setDayLimit(Integer.decode(dayLimit).intValue());
    } catch (NumberFormatException e) {
      logger.error("Invalid value for day limit: " + dayLimit);
    }
  }

  public String getForward() {
    return profile.getForward();
  }

  public void setForward(String forward) {
    profile.setForward(forward);
  }

  public String getMbDone() {
    return mbDone;
  }

  public void setMbDone(String mbDone) {
    this.mbDone = mbDone;
  }

  public String getMbCancel() {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }

  public boolean isCreate() {
    return create;
  }

  public void setCreate(boolean create) {
    this.create = create;
  }

  public String getMbDel() {
    return mbDel;
  }

  public void setMbDel(String mbDel) {
    this.mbDel = mbDel;
  }
}
