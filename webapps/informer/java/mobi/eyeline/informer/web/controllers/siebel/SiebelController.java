package mobi.eyeline.informer.web.controllers.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.siebel.SiebelManager;
import mobi.eyeline.informer.admin.siebel.impl.SiebelFinalStateListener;
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

  public SiebelController() {

    config = getConfig();

    Properties ps = config.getSiebelProperties();

    timeout = Integer.parseInt(ps.getProperty(SiebelManager.TIMEOUT));
    jdbcSource = ps.getProperty(SiebelManager.JDBC_SOURCE);
    jdbcLogin = ps.getProperty(SiebelManager.JDBC_USER);
    jdbcPassword = ps.getProperty(SiebelManager.JDBC_PASSWORD);
    dbType = ps.getProperty(SiebelManager.DB_TYPE);
    siebelUser = ps.getProperty(SiebelManager.USER);
    removeOnStop = Boolean.valueOf(ps.getProperty(SiebelManager.REMOVE_ON_STOP_PARAM));
    statsPeriod = Integer.parseInt(ps.getProperty(SiebelFinalStateListener.PERIOD_PARAM));

    if (!config.isSiebelDaemonStarted()) {
      ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", getLocale());
      error = bundle.getString("informer.siebel.daemon.offline");
    }
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
      if(!config.setSiebelProperties(getProperties(), getUserName())) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "informer.siebel.applying.not.started");
        return null;
      }
      return "INDEX";
    } catch (AdminException e) {
      addError(e);
      return null;
    }
  }

  private Properties getProperties() {
    Properties ps = config.getSiebelProperties();
    ps.setProperty(SiebelManager.TIMEOUT, Integer.toString(timeout));
    ps.setProperty(SiebelManager.JDBC_SOURCE, jdbcSource);
    ps.setProperty(SiebelManager.JDBC_USER, jdbcLogin);
    ps.setProperty(SiebelManager.JDBC_PASSWORD, jdbcPassword);
    ps.setProperty(SiebelManager.DB_TYPE, dbType);
    ps.setProperty(SiebelManager.USER, siebelUser);
    ps.setProperty(SiebelManager.REMOVE_ON_STOP_PARAM, Boolean.toString(removeOnStop));
    ps.setProperty(SiebelFinalStateListener.PERIOD_PARAM, Integer.toString(statsPeriod));
    return ps;
  }

  public String check() {
    if(config.checkSiebelProperties(getProperties())) {
      addLocalizedMessage(FacesMessage.SEVERITY_INFO, "informer.siebel.props.correct");
    }else {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "informer.siebel.props.illegal");
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
