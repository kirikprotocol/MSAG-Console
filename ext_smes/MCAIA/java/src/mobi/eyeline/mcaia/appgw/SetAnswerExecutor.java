package mobi.eyeline.mcaia.appgw;

import mobi.eyeline.mcaia.protocol.BusyRequest;
import mobi.eyeline.smpp.api.pdu.Message;
import mobi.eyeline.smpp.api.pdu.SubmitSM;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import mobi.eyeline.smpp.appgw.scenario.ExecutingException;
import mobi.eyeline.smpp.appgw.scenario.ExecutorResponse;
import mobi.eyeline.smpp.appgw.scenario.ScenarioInitializationException;
import mobi.eyeline.smpp.appgw.session.Session;

import java.sql.SQLException;
import java.util.Properties;

/**
 * Created by Serge Lugovoy
 * Date: May 27, 2010
 * Time: 2:56:48 PM
 */
public class SetAnswerExecutor extends ExecutorBase {
  String answer;
  String confirm;

  @Override
  public void init(Properties properties) throws ScenarioInitializationException {
    super.init(properties);    //To change body of overridden methods use File | Settings | File Templates.
    answer = getParam("answer");
    confirm = getParam("confirm");
  }

  public ExecutorResponse execute(Session session, Message message) throws ExecutingException, SQLException {
    ExecutorResponse resp = new ExecutorResponse(true, confirm);
    BusyRequest req = (BusyRequest) session.getAttribute("mcaReq");  // todo cool error processing
    SubmitSM msg = new SubmitSM();
    msg.setSourceAddress(session.getAbonent());
    try {
      msg.setDestinationAddress(req.getCaller());
    } catch (InvalidAddressFormatException e) {
      throw new ExecutingException("address"); // todo cool error processing
    }
    msg.setMessage(answer);
    resp.add(msg);
    return resp;
  }
}
