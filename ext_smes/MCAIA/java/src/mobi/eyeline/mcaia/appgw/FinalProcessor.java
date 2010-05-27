package mobi.eyeline.mcaia.appgw;

import mobi.eyeline.mcaia.StatusHandler;
import mobi.eyeline.mcaia.protocol.BusyRequest;
import mobi.eyeline.mcaia.protocol.Status;
import mobi.eyeline.smpp.api.pdu.Message;
import mobi.eyeline.smpp.appgw.scenario.ProcessingException;
import mobi.eyeline.smpp.appgw.scenario.ScenarioInitializationException;
import mobi.eyeline.smpp.appgw.scenario.ScenarioStateProcessor;
import mobi.eyeline.smpp.appgw.session.Session;

import java.util.Properties;

/**
 * Created by Serge Lugovoy
 * Date: May 26, 2010
 * Time: 3:49:00 PM
 */
public class FinalProcessor implements ScenarioStateProcessor {
  Status status;
  public void init(Properties properties) throws ScenarioInitializationException {
    status = Status.valueOf(properties.getProperty("status"));
  }

  public void process(Session session, Message message) throws ProcessingException {
    StatusHandler handler = (StatusHandler) session.getAttribute("mcaReqHandler");
    handler.handleStatus((BusyRequest) session.getAttribute("mcaReq"), status);
  }
}
