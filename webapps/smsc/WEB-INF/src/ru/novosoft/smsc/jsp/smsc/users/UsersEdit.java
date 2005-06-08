package ru.novosoft.smsc.jsp.smsc.users;

/**
 * Created by igork
 * Date: Nov 29, 2002
 * Time: 8:31:50 PM
 */

import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.Arrays;
import java.util.List;

public class UsersEdit extends UsersEditBean
{
	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK) return result;

		if (firstName == null || lastName == null)
		{
			if (login == null || login.trim().length() == 0)
			{
				setRoles(new String[0]);
				prefsNames = UserPreferences.getDefaultPrefsNames();
				prefsValues = UserPreferences.getDefaultPrefsValues();
				login = password = confirmPassword = firstName = lastName = dept = workPhone = homePhone = cellPhone = email = "";
				return error(SMSCErrors.error.users.loginNotDefined);
			}
			else
			{
				User user = userManager.getUser(login);
				if (user == null)
				{
					setRoles(new String[0]);
					prefsNames = UserPreferences.getDefaultPrefsNames();
					prefsValues = UserPreferences.getDefaultPrefsValues();
					password = confirmPassword = firstName = lastName = dept = workPhone = homePhone = cellPhone = email = "";
					return error(SMSCErrors.error.users.userNotFound, login);
				}
				else
				{
					password = "";
					confirmPassword = "";
					setRoles((String[]) user.getRoles().toArray(new String[0]));
					prefsNames  = UserPreferences.getDefaultPrefsNames();
					prefsValues = user.getPrefs().getPrefsValues();
					firstName   = user.getFirstName();
					lastName    = user.getLastName();
					dept        = user.getDept();
					workPhone   = user.getWorkPhone();
					homePhone   = user.getHomePhone();
					cellPhone   = user.getCellPhone();
					email       = user.getEmail();
				}
			}
		}
		return RESULT_OK;
	}

  protected int save(final HttpServletRequest request)
  {
    if (login == null || login.trim().length() == 0)
      return error(SMSCErrors.error.users.loginNotDefined);

    User user = userManager.getUser(login);
    if (user == null) { // add new user
      return error(SMSCErrors.error.users.userNotFound, login);
    }
    else {
      if ((password == null || password.trim().length() == 0) && (confirmPassword == null || confirmPassword.length() == 0)) {
        password = confirmPassword = user.getPassword().trim();
      }
      if (password == null || confirmPassword == null || !password.trim().equals(confirmPassword.trim()))
        return error(SMSCErrors.error.users.passwordNotConfirmed);

		user.setLogin(login);
		user.setPassword(password);
		user.setRoles(Arrays.asList(roles));
		user.setFirstName(firstName);
		user.setLastName(lastName);
		user.setDept(dept);
		user.setWorkPhone(workPhone);
		user.setHomePhone(homePhone);
		user.setCellPhone(cellPhone);
		user.setEmail(email);
		user.getPrefs().setValues(prefsNames, prefsValues);
      journalAppend(SubjectTypes.TYPE_user, login, Actions.ACTION_MODIFY);
      appContext.getStatuses().setUsersChanged(true);
      request.getSession().setAttribute("USER_LOGIN_ADD_EDIT", login);
      return RESULT_DONE;
    }
  }

  public boolean isNew()
  {
    return false;
  }

}
