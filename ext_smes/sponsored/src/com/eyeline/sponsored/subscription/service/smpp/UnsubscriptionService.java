package com.eyeline.sponsored.subscription.service.smpp;

import com.eyeline.sme.handler.services.BasicService;
import com.eyeline.sme.handler.SMPPServiceException;
import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.smpp.IncomingObject;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.sponsored.subscription.service.core.ProcessorException;
import com.eyeline.sponsored.subscription.service.core.SubscriptionProcessor;
import com.logica.smpp.Data;
import java.util.Properties;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;

/**
 *
 * @author artem
 */
public class UnsubscriptionService extends BasicService {

  private static final Category log = Category.getInstance("SUBSCRIPTION");  
  private String successResponse;  

  @Override
  protected void init(Properties initParams) throws SMPPServiceException {
    super.init(initParams);
    this.successResponse = initParams.getProperty("successResponse");    
    if (successResponse == null) {
      throw new SMPPServiceException("Success response does not specified");
    }    
  }

  public void serve(SMPPRequest request) {
    final String distributionName = request.getParameter("distributionName");
    if (distributionName == null) {
      log.error("Distribution name not specified in request");
      respond(request.getInObj(), Data.ESME_RX_P_APPN);
      return;
    }

    try {
      final Message reqMsg = request.getInObj().getMessage();

      if (log.isDebugEnabled()) {
        log.debug("Unsubscr req: subscr=" + reqMsg.getSourceAddress() + "; distr=" + distributionName);
      }
      // Unsubscribe
      SubscriptionProcessor.getInstance().unsubscribe(reqMsg.getSourceAddress(), distributionName);
      // Send response
      final Message respMsg = new Message();
      respMsg.setSourceAddress(reqMsg.getDestinationAddress());
      respMsg.setDestinationAddress(reqMsg.getSourceAddress());
      respMsg.setMessageString(successResponse);         
      try {
        send(respMsg);
      } catch (ShutdownedException e) {
        log.error("Send msg failed", e);
      }

      respond(request.getInObj(), Data.ESME_ROK);

    } catch (ProcessorException e) {
      log.error("Unsubscription failed", e);
      respond(request.getInObj(), Data.ESME_RX_P_APPN);
    }
  }

  private void respond(IncomingObject inObj, int code) {
    try {
      inObj.respond(code);
    } catch (SMPPException ex) {
      log.error("Can't send response", ex);
    }
  }  
}
