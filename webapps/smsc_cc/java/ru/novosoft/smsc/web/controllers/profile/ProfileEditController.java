package ru.novosoft.smsc.web.controllers.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.admin.profile.*;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.WebContext;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class ProfileEditController extends ProfileController {

  private static final List<SelectItem> reportOptions;
  private static final List<SelectItem> aliasHideOptions;

  static {
    reportOptions = new ArrayList<SelectItem>();
    reportOptions.add(new SelectItem(ReportOption.NONE));
    reportOptions.add(new SelectItem(ReportOption.FULL));
    reportOptions.add(new SelectItem(ReportOption.FINAL));

    aliasHideOptions = new ArrayList<SelectItem>();
    aliasHideOptions.add(new SelectItem(AliasHide.TRUE));
    aliasHideOptions.add(new SelectItem(AliasHide.FALSE));
    aliasHideOptions.add(new SelectItem(AliasHide.SUBSTITUTE));
  }

  private String profileAddr;
  private boolean newProfile;
  private String backPage;
  private Profile profile;
  protected boolean initFailed;

  public ProfileEditController() {
    profileAddr = getRequestParameter("profileAddr");
    backPage = getRequestParameter("backPage");

    try {
      if (profileAddr != null && profileAddr.length() > 0) {
        ProfileLookupResult res = mngr.lookupProfile(new Address(profileAddr));
        Profile p = res.getProfile();
        p.setAddress(new Address(profileAddr));
        setProfile(res.getProfile());
      } else {
        profile = new Profile();
        profile.setAddress(new Address(".0.0.0"));
        newProfile = true;
      }
    } catch (AdminException e) {
      addError(e);
      initFailed = true;
    }
  }

  public String getBackPage() {
    return backPage;
  }

  public void setBackPage(String backPage) {
    this.backPage = backPage;
  }

  public boolean isNewProfile() {
    return newProfile;
  }

  public void setNewProfile(boolean newProfile) {
    this.newProfile = newProfile;
  }

  public String getProfileAddress() {
    return profile.getAddress().getSimpleAddress();
  }

  public void setProfileAddress(String address)  {
    try {
      profile.setAddress(new Address(address));
    } catch (AdminException e) {
      addError(e);
    }
  }

  protected void setProfile(Profile profile) {
    this.profile = new ProfileWrapper(profile);
    newProfile = false;
  }

  public Profile getProfile() {
    return profile;
  }


  public boolean isInitFailed() {
    return initFailed;
  }

  public List<SelectItem> getReportOptions() {
    return reportOptions;
  }

  public List<SelectItem> getLocales() {
    List<SelectItem> res = new ArrayList<SelectItem>();
    try {
      for (String locale : WebContext.getInstance().getResourceManager().getLocales())
        res.add(new SelectItem(locale));
    } catch (AdminException e) {
      addError(e);
    }
    return res;
  }

  public List<SelectItem> getAliasHideOptions() {
    return aliasHideOptions;
  }

  public List<SelectItem> getClosedGroups() {
    List<SelectItem> res = new ArrayList<SelectItem>();
    res.add(new SelectItem(-1, null));
    try {
      for (ClosedGroup g : WebContext.getInstance().getClosedGroupManager().groups())
        res.add(new SelectItem(g.getId(), g.getName()));
    } catch (AdminException e) {
      addError(e);
    }
    return res;
  }

  public String submit() {
    if (getProfileAddress().length() == 0)
      return null;
    
    try {
      mngr.updateProfile(profile);
    } catch (AdminException e) {
      addError(e);
      return null;
    }

    return backPage;
  }

  public String reset() {
    return backPage;
  }

  public class ProfileWrapper extends Profile {

    public ProfileWrapper(Profile profile) {
      super(profile);
    }

    public void setGroupId(Integer id) {
      if (id == null || id == -1)
        id = null;

      super.setGroupId(id);
    }
  }
}
