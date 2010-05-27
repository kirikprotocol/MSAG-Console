package mobi.eyeline.mcaia.appgw;

import mobi.eyeline.mcaia.protocol.BusyRequest;
import mobi.eyeline.smpp.api.pdu.Message;
import mobi.eyeline.smpp.appgw.scenario.ExecutingException;
import mobi.eyeline.smpp.appgw.scenario.ExecutorResponse;
import mobi.eyeline.smpp.appgw.scenario.ScenarioError;
import mobi.eyeline.smpp.appgw.scenario.ScenarioInitializationException;
import mobi.eyeline.smpp.appgw.session.Session;

import java.sql.SQLException;
import java.text.MessageFormat;
import java.util.Properties;

/**
 * Created by Serge Lugovoy
 * Date: May 27, 2010
 * Time: 2:33:14 PM
 */
public class EntryExecutor extends ExecutorBase {
  MessageFormat text;
  @Override
  public void init(Properties properties) throws ScenarioInitializationException {
    super.init(properties);    //To change body of overridden methods use File | Settings | File Templates.
    text = new MessageFormat(getParam("text"));
  }

  public ExecutorResponse execute(Session session, Message message) throws ExecutingException, SQLException {
    BusyRequest req = (BusyRequest) message.getAppInfo("mcaReq");
    if( req == null ) throw new ExecutingException(ScenarioError.INVALID_REQUEST_TYPE);
    return new ExecutorResponse(false, text.format(new Object[]{req.getCaller()}));
  }
}
