package ru.sibinco.lib.backend.sme;

import org.apache.log4j.Logger;
import ru.sibinco.lib.SibincoException;

import java.util.NoSuchElementException;
import java.util.StringTokenizer;


/**
 * Created by igork
 * Date: 06.05.2003
 * Time: 16:58:10
 */
public class SmeStatus
{
  private Logger logger = Logger.getLogger(Class.class.getName());

  private String id;
  private boolean internal;
  private boolean connected;
  private byte bindMode;
  private String inAddress;
  private String outAddress;

  public SmeStatus(String id, boolean internal, boolean connected, byte bindMode, String inAddress, String outAddress)
  {
    this.id = id;
    this.internal = internal;
    this.connected = connected;
    this.bindMode = bindMode;
    this.inAddress = inAddress;
    this.outAddress = outAddress;
  }

  public SmeStatus(String initString) throws SibincoException
  {
    //logger.debug("init string: " + initString);
    StringTokenizer tokenizer = new StringTokenizer(initString, ",", false);
    try {
      id = tokenizer.nextToken();
      String connectStr = tokenizer.nextToken();
      if (connectStr.equalsIgnoreCase("internal")) {
        internal = true;
        connected = true;
        bindMode = Sme.MODE_TRX;
        inAddress = outAddress = "";
      } else {
        internal = false;
        if (connectStr.equalsIgnoreCase("tx")) {
          connected = true;
          bindMode = Sme.MODE_TX;
        } else if (connectStr.equalsIgnoreCase("rx")) {
          connected = true;
          bindMode = Sme.MODE_RX;
        } else if (connectStr.equalsIgnoreCase("trx")) {
          connected = true;
          bindMode = Sme.MODE_TRX;
        } else if (connectStr.equalsIgnoreCase("disconnected")) {
          connected = false;
          bindMode = Sme.MODE_UNKNOWN;
        } else {
          connected = true;
          bindMode = Sme.MODE_UNKNOWN;
        }

        if (connected) {
          inAddress = tokenizer.nextToken();
          outAddress = tokenizer.nextToken();
        } else {
          inAddress = outAddress = "";
        }
      }
    } catch (NoSuchElementException e) {
      logger.error("Init string misformatted \"" + initString + "\"", e);
      throw new SibincoException("Init string misformatted \"" + initString + "\"", e);
    }
    //logger.debug("result: "+id+"[int="+internal+",conn="+connected+",mode="+bindMode+",("+inAddress+"|"+outAddress+")");
  }

  public String getId()
  {
    return id;
  }

  public boolean isInternal()
  {
    return internal;
  }

  public boolean isConnected()
  {
    return connected;
  }

  public byte getBindMode()
  {
    return bindMode;
  }

  public String getInAddress()
  {
    return inAddress;
  }

  public String getOutAddress()
  {
    return outAddress;
  }
}
