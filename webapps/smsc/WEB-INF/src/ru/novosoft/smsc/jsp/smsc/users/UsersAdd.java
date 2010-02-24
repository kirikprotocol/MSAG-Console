package ru.novosoft.smsc.jsp.smsc.users;

/**
 * Created by igork
 * Date: Dec 2, 2002
 * Time: 4:57:25 PM
 */

import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.*;
import java.text.ParseException;

public class UsersAdd extends UsersEditBean {
  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    if (login == null || firstName == null || lastName == null) {
      login = "";
      password = "";
      confirmPassword = "";
      setRoles(new String[0]);
      prefsNames = UserPreferences.getDefaultPrefsNames();
      prefsValues = UserPreferences.getDefaultPrefsValues();
      firstName = "";
      lastName = "";
      dept = "";
      workPhone = "";
      homePhone = "";
      cellPhone = "";
      email = "";
    }
    return RESULT_OK;
  }

  protected int save(final HttpServletRequest request) {
    if (login == null || login.trim().length() == 0)
      return error(SMSCErrors.error.users.loginNotDefined);

    User user = userManager.getUser(login);
    if (user == null) {
      if (password == null || password.trim().length() == 0 || confirmPassword == null || confirmPassword.length() == 0)
        return error(SMSCErrors.error.users.passwordNotDefined);
      if (!password.trim().equals(confirmPassword.trim()))
        return error(SMSCErrors.error.users.passwordNotConfirmed);

      UserPreferences prefs = new UserPreferences();
      prefs.setValues(prefsNames, prefsValues);
      prefs.setInfoSmeAllowedRegions(getInfoSmeRegions(request));

      prefs.setInfosmeCacheSize(infosmeMessagesCacheSize);
      prefs.setInfosmeCacheSleep(infosmeMessagesCacheSleep);
      prefs.setInfosmeKeepHistory(infosmeKeepHistory);
      try{
        prefs.setInfosmePeriodEnd(tf.parse(infosmeActivePeriodEnd));
        prefs.setInfosmePeriodStart(tf.parse(infosmeActivePeriodStart));
        prefs.setInfosmeValidityPeriod(tf.parse(infosmeValidityPeriod));
      }catch(ParseException e) {
        logger.error(e,e);
        return error(e.getMessage());
      }
      prefs.setInfosmePriority(infosmePriority);
      prefs.setInfosmeReplaceMessage(infosmeReplaceMessage);
      prefs.setInfosmeSvcType(infosmeSvcType);
      prefs.setInfosmeTrackIntegrity(infosmeTrackIntegrity);
      prefs.setInfosmeTrMode(infosmeTransactionMode);
      prefs.setInfosmeUncommitGeneration(infosmeUncommitedInGeneration);
      prefs.setInfosmeUncommitProcess(infosmeUncommitedInProcess);
      prefs.setInfosmeWeekDaysSet(Arrays.asList(infosmeActiveWeekDays));
      if(isUssdPushFeature()) {
        prefs.setInfosmeUssdPush(Boolean.valueOf(infoSmeUssdPush));
      }else {
        prefs.setInfosmeUssdPush(null);
      }
      prefs.setInfosmeSourceAddress(infosmeSourceAddress);
      if (timezone != null)
        prefs.setTimezone(TimeZone.getTimeZone(timezone));
      try{
        if (userManager.addUser(new User(login, password.trim(), roles, firstName, lastName, dept, workPhone, homePhone, cellPhone, email, prefs))) {
          request.getSession().setAttribute("USER_LOGIN_ADD_EDIT", login);
          journalAppend(SubjectTypes.TYPE_user, login, Actions.ACTION_ADD);
          appContext.getStatuses().setUsersChanged(true);
          return RESULT_DONE;
        } else
          return error(SMSCErrors.error.unknown);
      }catch(AdminException e) {
        logger.error(e,e);
        return error(SMSCErrors.error.unknown);
      }
    } else {
      return error(SMSCErrors.error.users.userAlreadyExists, login);
    }
  }

  public boolean isNew() {
    return true;
  }
}
