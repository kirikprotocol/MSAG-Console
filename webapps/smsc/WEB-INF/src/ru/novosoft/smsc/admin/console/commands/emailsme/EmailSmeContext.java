package ru.novosoft.smsc.admin.console.commands.emailsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.utli.Proxy;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

public class EmailSmeContext extends Proxy {
  public static final byte LIMITTYPE_DAY = 0;
  public static final byte LIMITTYPE_WEEK = 0;
  public static final byte LIMITTYPE_MONTH = 0;

  public static final String LIMITCHAR_DAY_CHAR = "d";
  public static final String LIMITCHAR_DAY_WEEK = "w";
  public static final String LIMITCHAR_DAY_MONTH = "m";

  public static String EMAILSME_ID = "emailsme";
  private static final int STATUS_OK = 1;
  private static final int STATUS_DUBLICATE_USERNAME = 2;

  private static final byte COMMAND_UPDATE = 1;
  private static final byte COMMAND_DELETE = 2;
  private static final byte COMMAND_LOOKUP_BY_ADDR = 3;
  private static final byte COMMAND_LOOKUP_BY_USERID = 4;
  protected static final Object instanceLock = new Object();
  protected static EmailSmeContext instance = null;

  public static EmailSmeContext getInstance(SMSCAppContext appContext) throws AdminException {
    synchronized (instanceLock) {
      if (instance == null) {
        try {
          File servConfFolder = new File(appContext.getHostsManager().getServiceInfo(EMAILSME_ID).getServiceFolder(), "conf");
          Config config = new Config(new File(servConfFolder, "config.xml"));
          instance = new EmailSmeContext(config.getString("admin.host"), config.getInt("admin.port"));
        }
        catch (Exception e) {
          throw new AdminException("emailsme is not configured: " + e.getMessage());
        }
      }
      return instance;
    }
  }

  protected EmailSmeContext(String host, int port) {
    super(host, port);
    setTimeMode(false);
  }

  public synchronized void add(byte ton, byte npi, String address, String userName, String forwardEmail,
                  String realName, byte limitType, byte numberMapping, int lastLimitUpdateDate, int limitValue,
                  int limitCountGsm2Eml, int limitCountEml2Gsm) throws AdminException {
    checkConnect();
    try {
      int packetLength = 4/*commandId*/ + 1/*ton*/ + 1/*npi*/ + 21/*address*/ +
        2 + userName.length() + 2 + forwardEmail.length() + 2 + realName.length() + 1/*limitType*/ +
        1/*numberMapping*/ +
        4/*lastLimitUpdateDate*/ + 4/*limitValue*/ + 4/*limitCountGsm2Eml*/ + 4/*limitCountEml2Gsm*/;
      Message.writeUInt32(out, packetLength);
      Message.writeUInt32(out, COMMAND_UPDATE);
      Message.writeUInt8(out, ton);
      Message.writeUInt8(out, npi);
      byte[] addr = new byte[21];
      int len = (address.length() < 21 ? address.length() : 21);
      for (int i = 0; i < len; i++)
        addr[i] = (byte) address.charAt(i);

      out.write(addr);
      Message.writeString16(out, userName);
      Message.writeString16(out, forwardEmail);
      Message.writeString16(out, realName);
      Message.writeUInt8(out, limitType);
      Message.writeUInt8(out, numberMapping);
      Message.writeUInt32(out, lastLimitUpdateDate);
      Message.writeUInt32(out, limitValue);
      Message.writeUInt32(out, limitCountGsm2Eml);
      Message.writeUInt32(out, limitCountEml2Gsm);
      out.flush();
      int result = (int) Message.readUInt32(in);
      if (result == STATUS_DUBLICATE_USERNAME)
        throw new AdminException("Could not add: dublicate username");
      if (result != STATUS_OK) throw new AdminException("Could not add/update: emailsme returned error");
    }
    catch (java.io.IOException e) {
      throw new AdminException("Could not add/update: IOException");
    }
  }

  public synchronized void delete(byte ton, byte npi, String address) throws AdminException {
    checkConnect();
    try {
      int packetLength = 4/*commandId*/ + 1/*ton*/ + 1/*npi*/ + 21/*address*/;
      Message.writeUInt32(out, packetLength);
      Message.writeUInt32(out, COMMAND_DELETE);
      Message.writeUInt8(out, ton);
      Message.writeUInt8(out, npi);
      byte[] addr = new byte[21];
      int len = (address.length() < 21 ? address.length() : 21);
      for (int i = 0; i < len; i++) {
        addr[i] = (byte) address.charAt(i);
      }
      out.write(addr);
      out.flush();
      int result = (int) Message.readUInt32(in);

      if (result != STATUS_OK) throw new AdminException("Could not delete: emailsme returned error");
    }
    catch (java.io.IOException e) {
      throw new AdminException("Could not delete: IOException");
    }
  }

  public synchronized LookupResult lookupByAddress(byte ton, byte npi, String address) throws AdminException {
    checkConnect();
    try {
      int packetLength = 4/*commandId*/ + 1/*ton*/ + 1/*npi*/ + 21/*address*/;

      Message.writeUInt32(out, packetLength);
      Message.writeUInt32(out, COMMAND_LOOKUP_BY_ADDR);
      Message.writeUInt8(out, ton);
      Message.writeUInt8(out, npi);

      byte[] addr = new byte[21];
      int len = (address.length() < 21 ? address.length() : 21);
      for (int i = 0; i < len; i++) {
        addr[i] = (byte) address.charAt(i);
      }
      out.write(addr);

      return readResult(in);

    } catch (IOException e) {
      throw new AdminException("Could not lookup: IOException");
    }
  }

  public synchronized LookupResult lookupByUserId(String userId) throws AdminException {
    checkConnect();
    try {
      int packetLength = 4/*commandId*/ + 2 + userId.length() ;

      Message.writeUInt32(out, packetLength);
      Message.writeUInt32(out, COMMAND_LOOKUP_BY_USERID);
      Message.writeString16(out, userId);

      return readResult(in);

    } catch (IOException e) {
      throw new AdminException("Could not lookup: IOException");
    }
  }

  private static LookupResult readResult(InputStream in) throws IOException {
    final long resultLength = Message.readUInt32(in);
    if (resultLength == 0)
      return null;

    final LookupResult result = new LookupResult();

    result.setTon(readByte(in));
    result.setNpi(readByte(in));
    result.setAddr(Message.readString(in, 21));
    result.setUserName(Message.readString16(in));
    result.setForwardEmail(Message.readString16(in));
    result.setRealName(Message.readString16(in));
    result.setLimitType(readByte(in));
    result.setNumberMapping(readByte(in));
    result.setLastLimitUpdateDate(Message.readUInt32Int(in));
    result.setLimitValue(Message.readUInt32Int(in));
    result.setLimitCountGsm2Eml(Message.readUInt32Int(in));
    result.setLimitCountEml2Gsm(Message.readUInt32Int(in));

    return result;
  }

  private static byte readByte(InputStream in) throws IOException {
    final byte[] res = new byte[1];
    if (in.read(res) != -1)
      return res[0];
    throw new IOException("Input stream is over");
  }

  private void checkConnect() throws AdminException {
    disconnect();
    if (getStatus() != StatusConnected)
      connect(host, port);
  }
}
