package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundleException;
import ru.sibinco.smpp.appgw.util.Utils;
import ru.aurorisoft.smpp.Message;

import java.util.Properties;
import java.text.MessageFormat;

import org.apache.log4j.Category;

/**
 * Created by: Serge Lugovoy
 * Date: 06.09.2004
 * Time: 14:13:14
 */
public class SetRedirectExecutor implements Executor {
  private static Category logger = Category.getInstance(SetRedirectExecutor.class);
  DivertManager mgr;
  MessageFormat msgf;
  String type;

  public void init(Properties properties) throws ScenarioInitializationException {
    //To change body of implemented methods use File | Settings | File Templates.
    String type = properties.getProperty("type");
    ScenarioResourceBundle systemBundle = (ScenarioResourceBundle) properties.get("system");
    try {
      msgf = new MessageFormat( systemBundle.getString("divert_set"));
    } catch (ScenarioResourceBundleException e) {
      logger.error("", e);
    }
    mgr = DivertManager.getInstance();
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException {
    String msg = state.getMessageString();
    if( msg.equals("0") ) {
      // disable divert
    } else {
      // mgr.setUncond( state.getSourceAddress(), msg);
    }
    Object args [] = new Object[] {msg, state.getResourceBundle("system").getString("reason_"+type)};
    String msgresp = msgf.format(args);
    Message resp = new Message();
    resp.setMessageString(msgresp);
    return new ExecutorResponse(new Message[]{resp}, false);
  }
}
