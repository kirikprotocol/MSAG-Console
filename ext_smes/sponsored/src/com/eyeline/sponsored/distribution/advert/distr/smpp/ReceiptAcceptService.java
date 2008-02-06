package com.eyeline.sponsored.distribution.advert.distr.smpp;

import com.eyeline.sme.smppcontainer.SMPPService;
import com.eyeline.sme.smppcontainer.SMPPRequest;
import com.eyeline.sponsored.distribution.advert.distr.core.DeliveryStatsProcessor;
import com.eyeline.sponsored.distribution.advert.distr.core.ProcessorException;
import com.logica.smpp.Data;
import ru.aurorisoft.smpp.SMPPException;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 02.02.2008
 */

public class ReceiptAcceptService extends SMPPService{

  private static final Category log = Category.getInstance("RECEIPT");

  public void serve(SMPPRequest smppRequest) {
    try {
      smppRequest.getInObj().respond(Data.ESME_ROK);
    } catch (SMPPException e) {
      log.error(e, e);
    }

    final Message reqMsg = smppRequest.getInObj().getMessage();

    if (log.isDebugEnabled()) {
      log.debug("Delivery receipt req: subscr=" + reqMsg.getSourceAddress() + "; state=" + reqMsg.getMessageState());
    }

    if (reqMsg.isReceipt()) {
      if (reqMsg.getMessageState() == Message.MSG_STATE_DELIVERED) {
        try {
          DeliveryStatsProcessor.getInstance().registerDelivery(reqMsg.getSourceAddress(), 1);
        } catch (ProcessorException e) {
          log.error("Handle receipt failed", e);
        }
      }
    } else {
      log.error("Not receipt handled!");
    }
  }
}
