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
import mobi.eyeline.smsquiz.manager.SubscriptionManager;

/**
 * author: alkhal
 */
public class SubscriptionOffSMPPService extends BasicService {

  private static final Logger log = Logger.getLogger(SubscriptionOnSMPPService.class);

  private String responseText;
  private SubscriptionManager manager;


  protected void init(Properties properties) throws SMPPServiceException {
    super.init(properties);
    PropertiesConfig conf;
    try {
      conf = new PropertiesConfig(properties);
    } catch (ConfigException e) {
      throw new SMPPServiceException(e);
    }
    responseText = conf.getString("send.unsubscribe", "Success");

    manager = SubscriptionManager.getInstance();
  }

  public boolean serve(SMPPRequest request) {
    try {

      final Message reqMsg = request.getInObj().getMessage(),  respMsg = new Message();
      String clientAddress = reqMsg.getSourceAddress();

      respMsg.setSourceAddress(reqMsg.getDestinationAddress());
      respMsg.setDestinationAddress(clientAddress);

        try {
            manager.unsubscribe(clientAddress);
        } catch (Exception e) {
            log.error(e,e);
            return false;            
        }
        respMsg.setMessageString(responseText);

        request.getInObj().respond(Data.ESME_ROK);
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
