package ru.sibinco.scag.beans.gw.users;

import ru.sibinco.lib.backend.users.User;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.users.ScagUser;
import ru.sibinco.scag.backend.users.UserPreferences;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;


/**
 * Created by igork Date: 10.03.2004 Time: 17:55:16
 */
public class Edit extends EditBean {

    private static final String ALL_PROVIDERS = "ALL PROVIDERS";
    private static final String COLLAPSING_TREE_PARAM_PREFIX = "collapsing_tree_param.";
    protected String login = null;
    protected String password = null;
    protected String confirmPassword = null;
    protected String[] roles = null;
    protected long providerId = -1;
    protected String[] providerIds = new String[0];
    protected String[] providerNames = new String[0];
    protected String firstName = null;
    protected String lastName = null;
    protected String dept = null;
    protected String workPhone = null;
    protected String homePhone = null;
    protected String cellPhone = null;
    protected String email = null;
    protected Map params = new HashMap();
    protected Map requestParams = new HashMap();


    public String getId() {
        return login;
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        requestParams = request.getParameterMap();
        super.process(request, response);

        if (isAdd()) setDefaultValues();
      
        final Collection providers = appContext.getProviderManager().getProviders().values();
        final List ids = new ArrayList(providers.size());
        final List names = new ArrayList(providers.size());
        ids.add("-1");
        names.add(ALL_PROVIDERS);
        for (Iterator i = providers.iterator(); i.hasNext();) {
            final Provider provider = (Provider) i.next();
            ids.add(String.valueOf(provider.getId()));
            names.add(provider.getName());
        }
        providerIds = (String[]) ids.toArray(new String[0]);
        providerNames = (String[]) names.toArray(new String[0]);
    }

    protected void save() throws SCAGJspException {
        if ((null != password && 0 < getPassword().length()) || (null != confirmPassword && 0 < getConfirmPassword().length()))
            if (null != password ? !getPassword().equals(getConfirmPassword()) : !getConfirmPassword().equals(getPassword()))
                throw new SCAGJspException(Constants.errors.users.PASSWORD_NOT_CONFIRM);

        final Map users = appContext.getUserManager().getUsers();

        if (isAdd()) {
            if (null == login || 0 == getLogin().length())
                throw new SCAGJspException(Constants.errors.users.LOGIN_NOT_SPECIFIED);
            if (null == password || 0 == getPassword().length())
                throw new SCAGJspException(Constants.errors.users.PASSWORD_NOT_SPECIFIED);
            if (users.containsKey(getLogin()))
                throw new SCAGJspException(Constants.errors.users.USER_ALREADY_EXISTS, getLogin());
        } else {
            if (null == getEditId() || 0 == getEditId().length() || null == login || 0 == getLogin().length())
                throw new SCAGJspException(Constants.errors.users.LOGIN_NOT_SPECIFIED);
            if (users.containsKey(getLogin()) && !getEditId().equals(getLogin()))
                throw new SCAGJspException(Constants.errors.users.USER_ALREADY_EXISTS, getLogin());

            final User user = (User) users.remove(getEditId());
            if (null != user) {
                if (null == password || 0 == getPassword().length())
                    password = user.getPassword();
            }
        }

        for (Iterator i = requestParams.entrySet().iterator(); i.hasNext();) {
          Map.Entry entry = (Map.Entry) i.next();
          if (entry.getKey() instanceof String) {
              String key = (String) entry.getKey();
              if (key.startsWith(COLLAPSING_TREE_PARAM_PREFIX)) {
                  String name = key.substring(COLLAPSING_TREE_PARAM_PREFIX.length());
                  StringBuffer value = new StringBuffer();
                  for (int j = 0; j < ((String[]) entry.getValue()).length; j++) {
                      String valueElem = ((String[]) entry.getValue())[j];
                      value.append(valueElem.trim());
                  }                
                  params.put(name, value.toString());
              }
          }
        }

        users.put(getLogin(), new ScagUser(getLogin(), getPassword(), roles, getFirstName(), getLastName(),getDept(), getWorkPhone(), getHomePhone(), getCellPhone(), getEmail(), providerId, params));
        appContext.getStatuses().setUsersChanged(true);
        applyUsers();
        throw new DoneException();
    }

    protected void load(final String userLogin) throws SCAGJspException {
        if (null == userLogin || 0 == userLogin.length())
            throw new SCAGJspException(Constants.errors.users.LOGIN_NOT_SPECIFIED);

        if (!appContext.getUserManager().getUsers().containsKey(userLogin))
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, userLogin);

        final ScagUser user = (ScagUser) appContext.getUserManager().getUsers().get(userLogin);
        params = user.getPrefs().getMonPrefs();
        login = user.getLogin();
        roles = (String[]) user.getRoles().toArray(new String[0]);
        providerId = user.getProviderId();
        firstName = user.getFirstName();
        lastName = user.getLastName();
        dept = user.getDept();
        workPhone = user.getWorkPhone();
        homePhone = user.getHomePhone();
        cellPhone = user.getCellPhone();
        email = user.getEmail();
    }

    private void applyUsers() throws SCAGJspException {
        try {
            appContext.getUserManager().apply();
            appContext.getStatuses().setUsersChanged(false);
        } catch (Throwable e) {
            logger.debug("Couldn't apply users", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_USERS, e);
        }
    }

    private void setDefaultValues() {
        UserPreferences prefs = new UserPreferences();
        params.putAll(prefs.getMonPrefs());
    }

    public String getLogin() {
        if(login != null)login.trim();
        return login;
    }

    public void setLogin(final String login) {
        this.login = login;
    }

    public String getPassword() {
        if(password != null)password.trim();
        return password;
    }

    public void setPassword(final String password) {
        this.password = password;
    }

    public String getConfirmPassword() {
        if(confirmPassword != null)confirmPassword.trim();
        return confirmPassword;
    }

    public void setConfirmPassword(final String confirmPassword) {
        this.confirmPassword = confirmPassword;
    }

    public String[] getRoles() {
        return roles;
    }

    public void setRoles(final String[] roles) {
        this.roles = roles;
    }

    public String getFirstName() {
        if(firstName != null)firstName.trim();
        return firstName;
    }

    public void setFirstName(final String firstName) {
        this.firstName = firstName;
    }

    public String getLastName() {
        if(lastName != null)lastName.trim();
        return lastName;
    }

    public void setLastName(final String lastName) {
        this.lastName = lastName;
    }

    public String getDept() {
        if(dept != null)dept.trim();
        return dept;
    }

    public void setDept(final String dept) {
        this.dept = dept;
    }

    public String getWorkPhone() {
        if(workPhone != null)workPhone.trim();
        return workPhone;
    }

    public void setWorkPhone(final String workPhone) {
        this.workPhone = workPhone;
    }

    public String getHomePhone() {
        if(homePhone != null)homePhone.trim();
        return homePhone;
    }

    public void setHomePhone(final String homePhone) {
        this.homePhone = homePhone;
    }

    public String getCellPhone() {
        if(cellPhone != null)cellPhone.trim();
        return cellPhone;
    }

    public void setCellPhone(final String cellPhone) {
        this.cellPhone = cellPhone;
    }

    public String getEmail() {
        if(email != null)email.trim();
        return email;
    }

    public void setEmail(final String email) {
        this.email = email;
    }

    public String[] getProviderIds() {
        return providerIds;
    }

    public String[] getProviderNames() {
        return providerNames;
    }

    public long getProviderId() {
        return providerId;
    }

    public void setProviderId(final long providerId) {
        this.providerId = providerId;
    }

    public Map getParams() {
      return params;
    }

    public void setParams(Map params) {
      this.params = params;
    }
}
