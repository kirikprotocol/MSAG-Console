package mobi.eyeline.smsquiz.service;

import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.handler.SMPPServiceException;
import com.eyeline.sme.handler.services.BasicService;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import com.logica.smpp.Data;

import java.util.Properties;

import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;
import org.apache.log4j.Logger;

/**
 * author: alkhal
 */
// todo
public class SubscriptionHelpSMPPService extends BasicService {

  private static final Logger log = Logger.getLogger(SubscriptionHelpSMPPService.class);
  private String responseText;


  protected void init(Properties properties) throws SMPPServiceException {
    super.init(properties);
    PropertiesConfig conf;
    try {
      conf = new PropertiesConfig(properties);
    } catch (ConfigException e) {
      throw new SMPPServiceException(e);
    }
    responseText = conf.getString("send.request.wrong", "Wrong Request");
  }

  public boolean serve(SMPPRequest request) {
    try {

      request.getInObj().respond(Data.ESME_ROK);

      final Message reqMsg = request.getInObj().getMessage(),  respMsg = new Message();
      respMsg.setSourceAddress(reqMsg.getDestinationAddress());
      respMsg.setDestinationAddress(reqMsg.getSourceAddress());
      respMsg.setMessageString(responseText);
      try {
        send(respMsg);
      } catch (ShutdownedException e) {
        log.error("Shutdowned.", e);
      }

    } catch (SMPPException e) {
      log.error(e,e);
    }
    return true;
  }


}
