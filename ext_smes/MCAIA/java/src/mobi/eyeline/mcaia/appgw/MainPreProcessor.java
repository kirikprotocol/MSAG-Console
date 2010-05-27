package mobi.eyeline.mcaia.appgw;

import mobi.eyeline.smpp.api.pdu.Message;
import mobi.eyeline.smpp.appgw.scenario.ProcessingException;
import mobi.eyeline.smpp.appgw.scenario.ScenarioInitializationException;
import mobi.eyeline.smpp.appgw.scenario.ScenarioStateProcessor;
import mobi.eyeline.smpp.appgw.session.Session;

import java.util.Properties;

/**
 * Created by Serge Lugovoy
 * Date: May 26, 2010
 * Time: 3:13:23 PM
 */
public class MainPreProcessor implements ScenarioStateProcessor {
  public void init(Properties properties) throws ScenarioInitializationException {

  }

  public void process(Session session, Message message) throws ProcessingException {
    session.setAttribute("mcaReq", message.getAppInfo("mcaReq"));
    session.setAttribute("mcaReqHandler", message.getAppInfo("mcaReqHandler"));
  }
}
