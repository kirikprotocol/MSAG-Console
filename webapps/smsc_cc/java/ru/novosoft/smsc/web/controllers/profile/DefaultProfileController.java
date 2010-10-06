package ru.novosoft.smsc.web.controllers.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.admin.profile.AliasHide;
import ru.novosoft.smsc.admin.profile.Profile;
import ru.novosoft.smsc.admin.profile.ProfileManager;
import ru.novosoft.smsc.admin.profile.ReportOption;
import ru.novosoft.smsc.web.WebContext;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class DefaultProfileController extends ProfileController {

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

  private final ProfileManager mngr;
  private Profile profile;
  private boolean initFailed;

  public DefaultProfileController() {
    mngr = WebContext.getInstance().getProfileManager();
    try {
      profile = new ProfileWrapper(mngr.getDefaultProfile());
    } catch (AdminException e) {
      addError(e);
      initFailed = true;
    }
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
    try {
      mngr.updateDefaultProfile(profile);
    } catch (AdminException e) {
      addError(e);
      return null;
    }

    return "INDEX";
  }

  public class ProfileWrapper extends Profile {

    public ProfileWrapper(Profile profile) throws AdminException {
      super(profile);
    }

    public void setGroupId(Integer id) {
      if (id == null || id == -1)
        id = null;

      super.setGroupId(id);
    }
  }
}
