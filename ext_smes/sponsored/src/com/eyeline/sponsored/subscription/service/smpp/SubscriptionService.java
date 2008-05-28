package com.eyeline.sponsored.subscription.service.smpp;

import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.handler.SMPPServiceException;
import com.eyeline.sme.handler.services.BasicService;
import com.eyeline.sme.smpp.IncomingObject;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.sponsored.subscription.service.core.ProcessorException;
import com.eyeline.sponsored.subscription.service.core.SubscriptionProcessor;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;

import java.util.HashSet;
import java.util.Properties;
import java.util.Set;

/**
 *
 * @author artem
 */
public class SubscriptionService extends BasicService {

  private static final Category log = Category.getInstance("SUBSCRIPTION");  
  private String successResponse;  
  private String subscriptionClosedResponse;
  private String invalidVolumeResponse;  
  private Set<Integer> volumes;

  @Override
  protected void init(Properties initParams) throws SMPPServiceException {
    super.init(initParams);
    try {
      final PropertiesConfig conf = new PropertiesConfig(initParams);
      successResponse = conf.getString("successResponse");
      subscriptionClosedResponse = conf.getString("subscriptionClosedResponse");
      invalidVolumeResponse = conf.getString("invalidVolumeResponse");      
      volumes = new HashSet<Integer>(10);
      final int[] volumesArray = conf.getIntArray("volumes", ",");
      for (int i = 0; i < volumesArray.length; i++)
        volumes.add(volumesArray[i]);
    } catch (ConfigException e) {
      throw new SMPPServiceException(e);
    }    
  }

  public boolean serve(SMPPRequest request) {

    final String distributionName = request.getParameter("distributionName");
    if (distributionName == null) {
      log.error("Distribution name not specified in request");
      return false;
    }

    final String distributionVolume = request.getParameter("distributionVolume");
    if (distributionVolume == null) {
      log.error("Distribution volume not specified in request");
      return false;
    }

    int volume;
    try {
      volume = Integer.parseInt(distributionVolume);
    } catch (NumberFormatException e) {
      log.error("Invalid value of distribution volume: " + distributionVolume);
      return false;
    }

    try {
      final Message reqMsg = request.getInObj().getMessage();

      if (log.isDebugEnabled()) {
        log.debug("Subscr req: subscr=" + reqMsg.getSourceAddress() + "; distr=" + distributionName + "; vol=" + distributionVolume);
      }

      String responseText;
      if (!volumes.contains(volume)) {
        responseText = invalidVolumeResponse;

      } else {

        // Subscribe
        final SubscriptionProcessor.SubscriptionResult result = SubscriptionProcessor.getInstance().subscribe(reqMsg.getSourceAddress(), distributionName, volume);

        // Prepare response
        switch (result) {
          case SUCCESS:
            responseText = successResponse;
            break;
          default:
            responseText = subscriptionClosedResponse;
        }
      }

      // Send response
      final Message respMsg = new Message();
      respMsg.setSourceAddress(reqMsg.getDestinationAddress());
      respMsg.setDestinationAddress(reqMsg.getSourceAddress());
      respMsg.setMessageString(responseText);        
      try {
        send(respMsg);
      } catch (ShutdownedException e) {
        log.error("Send message failed", e);
      }

      respond(request.getInObj(), Data.ESME_ROK);
    } catch (ProcessorException e) {
      log.error("Subscription failed", e);
      return false;
    }

    return true;
  }

  private static void respond(IncomingObject inObj, int code) {
    try {
      inObj.respond(code);
    } catch (SMPPException ex) {
      log.error("Can't send response", ex);
    }
  }
}
