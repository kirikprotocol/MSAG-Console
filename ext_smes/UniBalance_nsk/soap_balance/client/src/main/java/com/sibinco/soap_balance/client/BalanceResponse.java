package com.sibinco.soap_balance.client;

import com.eyelinecom.whoisd.sptp.PDU;
import com.eyelinecom.whoisd.sptp.ProcessException;

import java.io.*;

import org.apache.log4j.Category;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class BalanceResponse extends Command {
  private final static Category logger = Category.getInstance(BalanceResponse.class);
  private String abonent;
  private double balance;

  public BalanceResponse() {
    super(Command.GET_BALANCE_RESP);
    for (int n = 1; n <= 2; n++) {
      pdu.addSegment(new byte[0]);
    }
  }

  public BalanceResponse(PDU pdu) throws ProcessException {
    super(pdu);
    parsePDU(pdu);
  }

  public void setPdu(PDU pdu) throws ProcessException {
    super.setPdu(pdu);
    parsePDU(pdu);
  }

  private void parsePDU(PDU pdu) throws ProcessException {
    if (pdu.getSegmentsCount() < 2) {
      throw new ProcessException("Illegal segments count: " + pdu.getSegmentsCount());
    }
    abonent = new String(pdu.getSegment(1));
    try
    {
      balance = (new DataInputStream(new ByteArrayInputStream(pdu.getSegment(2)))).readDouble();
    } catch(IOException during_getting_double)
    {
        throw new ProcessException("Error in segment #2: illegal double:", during_getting_double);
    }
  }

  public String getAbonent() {
    return abonent;
  }

  public void setAbonent(String abonent) {
    this.abonent = abonent;
    pdu.setSegment(1, abonent.getBytes());
  }

    public double getBalance()
    {
        return balance;
    }

    public void setBalance(double balance)
    {
        this.balance = balance;
        ByteArrayOutputStream output_stream = new ByteArrayOutputStream(4);
        DataOutputStream data_output_stream = new DataOutputStream(output_stream);
        try
        {
            data_output_stream.writeDouble(balance);
        } catch(IOException during_writing_double)
        {
            logger.warn("Can't write double value to ByteArrayOutputStream. Programmer error", during_writing_double);
        }
        pdu.setSegment(2, output_stream.toByteArray());
    }
}
