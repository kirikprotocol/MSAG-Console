package com.eyeline.sponsored.distribution.advert.distr.core;

import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sponsored.distribution.advert.distr.adv.BannerWithInfo;
import com.eyeline.sponsored.ds.banner.BannerMap;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SubmitResponse;

import java.util.Date;

/**
 * User: artem
 * Date: 22.04.2008
 */

final class OutgoingObjectWithBanner extends OutgoingObject  {

  private static final Category log = Category.getInstance("DISTRIBUTION");

  private final BannerMap bannerMap;
  private final int advertiserId;

  public OutgoingObjectWithBanner(String srcAddr, String dstAddr, BannerMap bannerMap, BannerWithInfo banner, Date validityPeriod) {
    this.bannerMap = bannerMap;
    this.advertiserId = banner.getAdvertiserId();

    final Message m = new Message();
    m.setSourceAddress(srcAddr);
    m.setDestinationAddress(dstAddr);
    m.setMessageString(banner.getBannerText());
    m.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);
    m.setValidityPeriod(validityPeriod);
    setMessage(m);
  }

  protected void handleResponse(PDU response) {
    if (response.getStatusClass() == PDU.STATUS_CLASS_NO_ERROR) {
      try {
        bannerMap.put(Long.parseLong(((SubmitResponse)response).getMessageId()), advertiserId);
      } catch (NumberFormatException e) {
        log.error(e,e);
      }
    }
  }
}
