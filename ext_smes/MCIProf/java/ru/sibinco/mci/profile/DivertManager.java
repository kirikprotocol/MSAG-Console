package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;

import java.io.*;
import java.util.Properties;
import java.util.StringTokenizer;
import java.net.Socket;

import org.apache.log4j.Category;

/**
 * Created by: Serge Lugovoy
 * Date: 06.09.2004
 * Time: 14:34:36
 */
public class DivertManager
{
  private static DivertManager instance = null;
  private static Object syncObj = new Object();

  private static Category logger = Category.getInstance(DivertManager.class);

  public static DivertManager getInstance() throws ScenarioInitializationException
  {
    synchronized(syncObj) {
      return (instance == null) ? (instance = new DivertManager()):instance;
    }
  }

  private int    mscPort = -1;
  private String mscHost          = null;
  private String mscUserCode      = null;
  private String mscUserPassword  = null;
  private String mscNvtIODevice   = null;
  private String mciSmeAddress    = null;
  private String voiceMailAddress = null;

  private InputStream  is = null;
  private OutputStream os = null;
  private Socket mscSocket = null;
  private Object mscSocketLock = new Object();

  protected DivertManager() throws ScenarioInitializationException
  {
    InputStream is = this.getClass().getClassLoader().getResourceAsStream(Constants.MCI_PROF_MSC_FILE);
    if (is == null)
      throw new ScenarioInitializationException("Failed to locate commutator properties file");

    try {
      Properties properties = new Properties();
      properties.load(is);

      mscPort          = Integer.decode(properties.getProperty("MSC.port")).intValue();
      mscHost          = properties.getProperty("MSC.host");
      mscNvtIODevice   = properties.getProperty("MSC.nvtIODevice");
      mscUserCode      = properties.getProperty("MSC.usercode");
      mscUserPassword  = properties.getProperty("MSC.userpassword");
      voiceMailAddress = properties.getProperty("MSC.voicemail");
      mciSmeAddress    = properties.getProperty("MCISme.Address");

    } catch(Exception e) {
      final String err = "Failed to load commutator properties";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    } finally {
      try { is.close(); } catch (Throwable th) {}
    }
  }

  private final static int ESC_IAC = 255;
  private final static int ESC_SB  = 250;
  private final static int ESC_SE  = 240;
  private final static int ESC_ESC = 27;
  private final static int ESC_CR  = 13;
  private final static int ESC_LF  = 10;
  private final static int ESC_BS  = 8;
  private final static int ESC_NUL = 0;

  private String readTelnetString(int stopChar) throws IOException
  {
    int b = -1;
    StringBuffer sb = new StringBuffer();
    boolean escape = false; boolean parameter = false; boolean typeofop = false;

    while ((b=is.read()) != -1)
    {
      //System.out.println("Got CHR="+(char)b+" code "+b);
      if (b == ESC_IAC) {
        //System.out.println("Got IAC, esc: "+escape);
        if (parameter) continue;
        if (!escape) {
          escape = true; continue;
        } else escape = false;
      }
      if (typeofop) {
        //System.out.println("Got TGT="+b);
        escape = false; typeofop = false; continue;
      }
      if (parameter) {
        if (b == ESC_SE) {
          //System.out.println("Got SE");
          escape = false; parameter = false;
        }
        continue;
      }
      if (escape) {
        if (b == ESC_SB) parameter = true;
        else typeofop = true;
        continue;
      }
      if (b == ESC_ESC) {
        if ((b = is.read()) == -1 ) break; // skip 91
        //System.out.println("Got CHR="+(char)b+" code "+b);
        if ((b = is.read()) == -1 ) break; // skip opcode
        //System.out.println("Got CHR="+(char)b+" code "+b);
        if (b >= 49 && b <= 54) // for Ins, Del, Home, End, PgUp, PgDn
            if ((b = is.read()) == -1 ) break;
        continue;
      }
      if (b == ESC_BS) {
        if (sb.length() > 0) sb.deleteCharAt(sb.length()-1);
        continue;
      }

      if (b == ESC_NUL || b == stopChar) break;
      if (b == ESC_CR) continue;
      byte bytes[] = {(byte)b};
      sb.append(new String(bytes, System.getProperty("file.encoding")));
    }
    //if (b == -1) throw new SocketException("End of stream reached");
    return sb.toString();
  }
  private void writeTelnetString(String str) throws IOException
  {
    byte data[] = str.getBytes();
    for (int i=0; i<data.length; i++) {
      if (data[i] == -1 || data[i] == ESC_IAC) {
        os.write(ESC_IAC); os.write(ESC_IAC);
      } else os.write(data[i]);
      //System.out.println("Output: '"+data[i]+"' simbol !");
    }
    os.flush();
  }
  private String readTelnetLine() throws IOException {
    return readTelnetString(ESC_LF);
  }
  private void writeTelnetLine(String str) throws IOException {
    writeTelnetString(str+"\r\n");
  }

  private final static int ESC_SEMI   = ':';
  private final static int ESC_PROMPT = '<';

  private void connect() throws DivertManagerException, IOException
  {
    if (mscSocket == null || !(mscSocket.isConnected()))
    {
      try {
        if (mscSocket != null) {
          if (is != null) is.close(); if (os != null) os.close();
          mscSocket.close(); mscSocket = null;
        }
        logger.info("Connecting to MSC "+mscHost+":"+mscPort+"...");
        mscSocket = new Socket(mscHost, mscPort);
        is = mscSocket.getInputStream(); os = mscSocket.getOutputStream();

        // login using params MSC.nvtIODevice, MSC.USERCODE, MSC.PASSWORD
        logger.info("Connected Ok");
        readTelnetString(ESC_SEMI); writeTelnetLine(mscNvtIODevice);
        readTelnetString(ESC_SEMI); writeTelnetLine(mscUserCode);
        readTelnetString(ESC_SEMI); writeTelnetLine(mscUserPassword);
        readTelnetString(ESC_PROMPT);
        logger.info("Autentificated user="+mscUserCode);
      } catch (IOException exc) {
        logger.error("Connect to MSC "+mscHost+":"+mscPort+" error", exc);
        throw new DivertManagerException(exc, DivertManagerException.CONNECT);
      }
    }
    else if (is.available() > 0)
    {
        while (is.available() > 0 && (is.read() != -1)); // Skip TIME OUT string
        writeTelnetLine(""); readTelnetString(ESC_PROMPT);
    }
  }

  private final static String COMMAND_SET = "HGSSI";
  private final static String COMMAND_DEL = "HGSSE";
  private final static String COMMAND_GET = "HGSDP";

  private final static String REASON_NOT_AVAIL = "CFNRC";
  private final static String REASON_ABSENT    = "CFNRY";
  private final static String REASON_UNCOND    = "CFU";
  private final static String REASON_BUSY      = "CFB";

  private final static String MSISDN_STR    = ":MSISDN=";
  private final static String RESPONCE_OK   = "EXECUTED";
  private final static String RESPONCE_FAIL = "NOT ACCEPTED";

  // set divert for abonent=msisdn for reason=ss to address
  private void set(String msisdn, String ss, String address) throws DivertManagerException, IOException
  {
    // HGSSI:MSISDN=msisdn,SS=ss,FNUM=address;
    String command = COMMAND_SET + MSISDN_STR + checkAndConvertAddress(msisdn) +
                     ",SS=" + ss + ",FNUM=" + address + ";";
    logger.info("Sending command: "+command);
    writeTelnetLine(command);
    logger.info("Command sent");
    String responce = readTelnetString(ESC_PROMPT);
    logger.info("Got responce: "+responce);
    if (responce == null || responce.length() <= 0 || !responce.startsWith(RESPONCE_OK))
      throw new DivertManagerException("Set divert settings failed", DivertManagerException.NOT_ACCEPTED);
  }
  // clear divert for abonent=msisdn for reason=ss
  private void del(String msisdn, String ss) throws DivertManagerException, IOException
  {
    // HGSSE:MSISDN=msisdn,SS=ss,KEEP;
    String command = COMMAND_DEL + MSISDN_STR + checkAndConvertAddress(msisdn) +
                     ",SS=" + ss + ",KEEP;";
    logger.info("Sending command: "+command);
    writeTelnetLine(command);
    logger.info("Command sent");
    String responce = readTelnetString(ESC_PROMPT);
    logger.info("Got responce: "+responce);
    if (responce == null || responce.length() <= 0 || !responce.startsWith(RESPONCE_OK))
      throw new DivertManagerException("Del divert settings failed", DivertManagerException.NOT_ACCEPTED);
  }

  private final static String SSD_STR       = "SUPPLEMENTARY SERVICE DATA";
  private final static String ACTIVE_OP_STR = "ACTIVE-OP";

  private String divertToLocal(String divert)
  {
    if (divert == null || divert.length() <= 0) return Constants.OFF;
    divert = divert.trim();
    if (divert.equalsIgnoreCase(mciSmeAddress)) return Constants.SERVICE;
    if (divert.equalsIgnoreCase(voiceMailAddress)) return Constants.VOICEMAIL;
    String _divert = "+"+divert;
    if (_divert.equalsIgnoreCase(mciSmeAddress)) return Constants.SERVICE;
    if (_divert.equalsIgnoreCase(voiceMailAddress)) return Constants.VOICEMAIL;
    return divert;
  }
  private String localToDivert(String local)  throws IOException
  {
    if (local.equalsIgnoreCase(Constants.SERVICE)) local = mciSmeAddress;
    else if (local.equalsIgnoreCase(Constants.VOICEMAIL)) local = voiceMailAddress;
    return checkAndConvertAddress(local);
  }
  private String checkAndConvertAddress(String address) throws IOException
  {
    if (address == null || address.length() <= 0)
      throw new IOException("Abonent address is undefined");
    return (address.startsWith("+")) ? address.substring(1):address;
  }

  public DivertInfo getDivertInfo(String abonent) throws DivertManagerException
  {
    synchronized(mscSocketLock)
    {
      try
      {
        connect();
        String command = COMMAND_GET + MSISDN_STR + checkAndConvertAddress(abonent) + ",SSDA;";

        logger.info("Sending command: "+command);
        writeTelnetLine(command);
        logger.info("Command sent");
        String responce = readTelnetString(ESC_PROMPT);
        logger.info("Got responce: "+responce);
        if (responce == null || responce.length() <= 0 || responce.trim().startsWith(RESPONCE_FAIL))
          throw new DivertManagerException("Get divert settings failed", DivertManagerException.NOT_ACCEPTED);

        int index = responce.indexOf(SSD_STR);
        if (index < 0)
          throw new IOException("Failed to locate SS data in responce: "+responce);
        responce = responce.substring(index + SSD_STR.length()).trim();

        DivertInfo info = new DivertInfo();
        StringTokenizer st = new StringTokenizer(responce, "\r\n");
        while (st.hasMoreTokens())
        {
          String line = st.nextToken();
          StringTokenizer linest = new StringTokenizer(line);
          if (linest.countTokens() > 2)
          {
            String ss = linest.nextToken();      // ss
            String status = linest.nextToken();  // status
            if (!status.startsWith(ACTIVE_OP_STR)) continue;
            String address = linest.nextToken(); // fnum (forwarded to address)
            if      (ss.startsWith(REASON_BUSY))      info.setBusy(divertToLocal(address));
            else if (ss.startsWith(REASON_UNCOND))    info.setUncond(divertToLocal(address));
            else if (ss.startsWith(REASON_ABSENT))    info.setAbsent(divertToLocal(address));
            else if (ss.startsWith(REASON_NOT_AVAIL)) info.setNotavail(divertToLocal(address));
          }
        }
        return new DivertInfo(info);
      }
      catch (IOException exc) {
        logger.error("Communication with MSC error", exc);
        throw new DivertManagerException(exc, DivertManagerException.COMMUNICATION);
      }
    }
  }

  private void setDivert(String abonent, String ss, String divert)
      throws DivertManagerException, IOException
  {
    if (divert == null || divert.equalsIgnoreCase(Constants.OFF)) del(abonent, ss);
    else set(abonent, ss, localToDivert(divert));
  }
  public void setDivertInfo(String abonent, DivertInfo info)
      throws DivertManagerException
  {
    synchronized(mscSocketLock)
    {
      try
      {
        connect();
        if (info.isAbsentChanged()) {
          setDivert(abonent, REASON_ABSENT, info.getAbsent());
          info.clearAbsent();
        }
        if (info.isBusyChanged()) {
          setDivert(abonent, REASON_BUSY, info.getBusy());
          info.clearBusy();
        }
        if (info.isNotavailChanged()) {
          setDivert(abonent, REASON_NOT_AVAIL, info.getNotavail());
          info.clearNotavail();
        }
        if (info.isUncondChanged()) {
          setDivert(abonent, REASON_UNCOND, info.getUncond());
          info.clearUncond();
        }
      }
      catch (IOException exc) {
        logger.error("Communication with MSC error", exc);
        throw new DivertManagerException(exc, DivertManagerException.COMMUNICATION);
      }
    }
  }
}
