package mobi.eyeline.informer.web.controllers.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.siebel.SiebelManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.LinkedList;
import java.util.List;
import java.util.Properties;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelController extends InformerController {

  private int timeout;

  private String siebelUser;

  private String jdbcSource;

  private String jdbcLogin;

  private String jdbcPassword;

  private String jdbcDriver;

  private Configuration config;

  private boolean removeOnStop;

  private String error;

  public SiebelController() {

    config = getConfig();

    Properties ps = config.getSiebelProperties();

    timeout = Integer.parseInt(ps.getProperty(SiebelManager.TIMEOUT));
    jdbcSource = ps.getProperty(SiebelManager.JDBC_SOURCE);
    jdbcLogin = ps.getProperty(SiebelManager.JDBC_USER);
    jdbcPassword = ps.getProperty(SiebelManager.JDBC_PASSWORD);
    jdbcDriver = ps.getProperty(SiebelManager.JDBC_DRIVER);
    siebelUser = ps.getProperty(SiebelManager.USER);
    removeOnStop = Boolean.valueOf(ps.getProperty(SiebelManager.REMOVE_ON_STOP_PARAM));

    if(!config.isSiebelDaemonStarted()) {
      ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", getLocale());
      error = bundle.getString("informer.siebel.daemon.offline");
    }
  }

  public String getError() {
    return error;
  }

  public List<SelectItem> getUniqueUsers() {
    List<SelectItem> res = new LinkedList<SelectItem>();
    for(User u : config.getUsers()) {
      if(u.hasRole("informer-admin")){
        res.add(new SelectItem(u.getLogin(), u.getLogin()));
      }
    }
    return res;
  }

  public String save() {

    try {
      Class.forName(jdbcDriver);
    } catch (Exception e) {
      logger.warn(e,e);
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "informer.siebel.jdbc.driver.illegal", jdbcDriver);
      return null;
    }

    try{
      Properties ps = config.getSiebelProperties();
      ps.setProperty(SiebelManager.TIMEOUT, Integer.toString(timeout));
      ps.setProperty(SiebelManager.JDBC_SOURCE, jdbcSource);
      ps.setProperty(SiebelManager.JDBC_USER, jdbcLogin);
      ps.setProperty(SiebelManager.JDBC_PASSWORD, jdbcPassword);
      ps.setProperty(SiebelManager.JDBC_DRIVER, jdbcDriver);
      ps.setProperty(SiebelManager.USER, siebelUser);
      ps.setProperty(SiebelManager.REMOVE_ON_STOP_PARAM, Boolean.toString(removeOnStop));
      config.setSiebelProperties(ps, getUserName());
    }catch (AdminException e){
      addError(e);
      return null;
    }
    return "INDEX";
  }

  public String getJdbcDriver() {
    return jdbcDriver;
  }

  public void setJdbcDriver(String jdbcDriver) {
    this.jdbcDriver = jdbcDriver;
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
