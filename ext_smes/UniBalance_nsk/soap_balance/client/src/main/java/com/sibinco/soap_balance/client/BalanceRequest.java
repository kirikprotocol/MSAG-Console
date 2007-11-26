package com.sibinco.soap_balance.client;

import com.eyelinecom.whoisd.sptp.PDU;
import com.eyelinecom.whoisd.sptp.ProcessException;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class BalanceRequest extends Command {
  private String abonent;

  public BalanceRequest() {
    super(Command.GET_BALANCE);
    for (int n = 1; n <= 1; n++) {
      pdu.addSegment(new byte[0]);
    }
  }

  public BalanceRequest(PDU pdu) throws ProcessException {
    super(pdu);
    parsePDU(pdu);
  }

  public void setPdu(PDU pdu) throws ProcessException {
    super.setPdu(pdu);
    parsePDU(pdu);
  }

  private void parsePDU(PDU pdu) throws ProcessException {
    if (pdu.getSegmentsCount() < 1) {
      throw new ProcessException("Illegal segments count: " + pdu.getSegmentsCount());
    }
    abonent = new String(pdu.getSegment(1));
  }

  public String getAbonent() {
    return abonent;
  }

  public void setAbonent(String abonent) {
    this.abonent = abonent;
    pdu.setSegment(1, abonent.getBytes());
  }
}
