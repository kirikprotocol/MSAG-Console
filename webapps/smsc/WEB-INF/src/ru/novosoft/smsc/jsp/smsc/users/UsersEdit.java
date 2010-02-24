package ru.novosoft.smsc.jsp.smsc.users;

/**
 * Created by igork
 * Date: Nov 29, 2002
 * Time: 8:31:50 PM
 */

import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.*;
import java.text.ParseException;

public class UsersEdit extends UsersEditBean {
  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    if (firstName == null || lastName == null) {
      if (login == null || login.trim().length() == 0) {
        setRoles(new String[0]);
        prefsNames = UserPreferences.getDefaultPrefsNames();
        prefsValues = UserPreferences.getDefaultPrefsValues();
        login = password = confirmPassword = firstName = lastName = dept = workPhone = homePhone = cellPhone = email = "";
        return error(SMSCErrors.error.users.loginNotDefined);
      } else {
        User user = userManager.getUser(login);
        if (user == null) {
          setRoles(new String[0]);
          prefsNames = UserPreferences.getDefaultPrefsNames();
          prefsValues = UserPreferences.getDefaultPrefsValues();
          password = confirmPassword = firstName = lastName = dept = workPhone = homePhone = cellPhone = email = "";
          return error(SMSCErrors.error.users.userNotFound, login);
        } else {
          password = "";
          confirmPassword = "";
          setRoles((String[]) user.getRoles().toArray(new String[0]));
          prefsNames = UserPreferences.getDefaultPrefsNames();
          prefsValues = user.getPrefs().getPrefsValues();
          timezone = user.getPrefs().getTimezone().getID();
          firstName = user.getFirstName();
          lastName = user.getLastName();
          dept = user.getDept();
          workPhone = user.getWorkPhone();
          homePhone = user.getHomePhone();
          cellPhone = user.getCellPhone();
          email = user.getEmail();

          infosmePriority = user.getPrefs().getInfosmePriority();
          infosmeValidityPeriod = tf.format(user.getPrefs().getInfosmeValidityPeriod());
          infosmeReplaceMessage = user.getPrefs().isInfosmeReplaceMessage();
          infosmeSvcType = user.getPrefs().getInfosmeSvcType();
          infosmeActivePeriodStart = tf.format(user.getPrefs().getInfosmePeriodStart());
          infosmeActivePeriodEnd = tf.format(user.getPrefs().getInfosmePeriodEnd());
          infoSmeUssdPush = user.getPrefs().isInfosmeUssdPush() != null && user.getPrefs().isInfosmeUssdPush().booleanValue();
          infosmeSourceAddress = user.getPrefs().getInfosmeSourceAddress();
          Collection set = user.getPrefs().getInfosmeWeekDaysSet();
          infosmeActiveWeekDays = new String[set.size()];
          int i=0;
          for (Iterator iter = set.iterator(); iter.hasNext();) {
            infosmeActiveWeekDays[i] = (String)iter.next();
            i++;
          }

          infosmeMessagesCacheSize = user.getPrefs().getInfosmeCacheSize();
          infosmeMessagesCacheSleep = user.getPrefs().getInfosmeCacheSleep();
          infosmeTransactionMode = user.getPrefs().isInfosmeTrMode();
          infosmeUncommitedInGeneration = user.getPrefs().getInfosmeUncommitGeneration();
          infosmeUncommitedInProcess = user.getPrefs().getInfosmeUncommitProcess();
          infosmeTrackIntegrity = user.getPrefs().isInfosmeTrackIntegrity();
          infosmeKeepHistory = user.getPrefs().isInfosmeKeepHistory();
        }
      }
    }
    return RESULT_OK;
  }

  protected int save(final HttpServletRequest request) {
    if (login == null || login.trim().length() == 0)
      return error(SMSCErrors.error.users.loginNotDefined);

    User user = userManager.getUser(login);
    if (user == null) { // add new user
      return error(SMSCErrors.error.users.userNotFound, login);
    } else {
      if ((password == null || password.trim().length() == 0) && (confirmPassword == null || confirmPassword.length() == 0)) {
        password = confirmPassword = user.getPassword().trim();
      }
      if (password == null || confirmPassword == null || !password.trim().equals(confirmPassword.trim()))
        return error(SMSCErrors.error.users.passwordNotConfirmed);

      user.setLogin(login);
      user.setPassword(password);
      user.setRoles(Arrays.asList(roles == null ? new String[0] : roles));
      user.setFirstName(firstName);
      user.setLastName(lastName);
      user.setDept(dept);
      user.setWorkPhone(workPhone);
      user.setHomePhone(homePhone);
      user.setCellPhone(cellPhone);
      user.setEmail(email);

      UserPreferences prefs = user.getPrefs();

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
      prefs.setInfosmeSourceAddress(infosmeSourceAddress);
      if(isUssdPushFeature()) {
        prefs.setInfosmeUssdPush(Boolean.valueOf(infoSmeUssdPush));
      }
      if (timezone != null)
        prefs.setTimezone(TimeZone.getTimeZone(timezone));
      journalAppend(SubjectTypes.TYPE_user, login, Actions.ACTION_MODIFY);
      appContext.getStatuses().setUsersChanged(true);
      request.getSession().setAttribute("USER_LOGIN_ADD_EDIT", login);
      return RESULT_DONE;
    }
  }

  public boolean isNew() {
    return false;
  }

}
