package mobi.eyeline.informer.web.controllers.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.siebel.SiebelSettings;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelController extends InformerController {

  private int timeout;

  private String siebelUser;

  private String jdbcSource;

  private String jdbcLogin;

  private String jdbcPassword;

  private String dbType;

  private final Configuration config;

  private boolean removeOnStop;

  private String error;

  private int statsPeriod;

  private boolean init;

  public SiebelController() {

    config = getConfig();

    if(getRequestParameter("siebel_init") == null) {

      SiebelSettings ps = config.getSiebelSettings();

      timeout = ps.getTimeout();
      jdbcSource = ps.getJdbcSource();
      jdbcLogin = ps.getJdbcLogin();
      jdbcPassword = ps.getJdbcPassword();
      dbType = ps.getDbType();
      siebelUser = ps.getUser();
      removeOnStop = ps.isRemoveOnStop();
      statsPeriod = ps.getStatsPeriod();

      init = true;
    }
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }

  public List<SelectItem> getUniqueDBTypes() {
    List<SelectItem> result = new ArrayList<SelectItem>(2);
    result.add(new SelectItem("mysql", "mysql"));
    result.add(new SelectItem("oracle", "oracle"));
    return result;
  }

  public String getError() {
    return error;
  }

  public List<SelectItem> getUniqueUsers() {
    List<SelectItem> res = new LinkedList<SelectItem>();
    for (User u : config.getUsers()) {
      if (u.hasRole("informer-admin")) {
        res.add(new SelectItem(u.getLogin(), u.getLogin()));
      }
    }
    Collections.sort(res, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return res;
  }

  public int getStatsPeriod() {
    return statsPeriod;
  }

  public void setStatsPeriod(int statsPeriod) {
    this.statsPeriod = statsPeriod;
  }

  public String save() {
    try {
      if(!config.setSiebelSettings(getSettings(), getUserName())) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "informer.siebel.applying.not.started");
        return null;
      }
      return "INDEX";
    } catch (AdminException e) {
      addError(e);
      return null;
    }
  }

  private SiebelSettings getSettings() {
    SiebelSettings ps = config.getSiebelSettings();
    ps.setTimeout(timeout);
    ps.setJdbcSource(jdbcSource);
    ps.setJdbcLogin(jdbcLogin);
    ps.setJdbcPassword(jdbcPassword);
    ps.setDbType(dbType);
    ps.setUser(siebelUser);
    ps.setRemoveOnStop(removeOnStop);
    ps.setStatsPeriod(statsPeriod);
    return ps;
  }

  public String check() {
    try {
      if(config.checkSiebelSettings(getSettings())) {
        addLocalizedMessage(FacesMessage.SEVERITY_INFO, "informer.siebel.props.correct");
      }else {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "informer.siebel.props.illegal");
      }
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String getDbType() {
    return dbType;
  }

  public void setDbType(String dbType) {
    this.dbType = dbType;
  }

  public int getTimeout() {
    return timeout;
  }

  public void setTimeout(int timeout) {
    this.timeout = timeout;
  }

  public String getSiebelUser() {
    return siebelUser;
  }

  public void setSiebelUser(String siebelUser) {
    this.siebelUser = siebelUser;
  }

  public String getJdbcSource() {
    return jdbcSource;
  }

  public void setJdbcSource(String jdbcSource) {
    this.jdbcSource = jdbcSource;
  }

  public String getJdbcLogin() {
    return jdbcLogin;
  }

  public void setJdbcLogin(String jdbcLogin) {
    this.jdbcLogin = jdbcLogin;
  }

  public String getJdbcPassword() {
    return jdbcPassword;
  }

  public void setJdbcPassword(String jdbcPassword) {
    this.jdbcPassword = jdbcPassword;
  }

  public boolean isRemoveOnStop() {
    return removeOnStop;
  }

  public void setRemoveOnStop(boolean removeOnStop) {
    this.removeOnStop = removeOnStop;
  }
}
