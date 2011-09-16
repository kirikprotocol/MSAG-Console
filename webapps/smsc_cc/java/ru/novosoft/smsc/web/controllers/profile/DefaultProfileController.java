package ru.novosoft.smsc.web.controllers.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profile.AliasHide;
import ru.novosoft.smsc.admin.profile.ReportOption;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class DefaultProfileController extends ProfileEditController {

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

  public DefaultProfileController() {
    try {
      setProfile(new ProfileWrapper(mngr.getDefaultProfile()));
    } catch (AdminException e) {
      addError(e);
      initFailed = true;
    }
  }

  public String submit() {
    try {
      mngr.updateDefaultProfile(getProfile());
    } catch (AdminException e) {
      addError(e);
      return null;
    }

    return "INDEX";
  }

  public String reset() {
    return "DEFAULT_PROFILE";
  }

}
