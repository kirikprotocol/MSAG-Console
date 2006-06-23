package ru.novosoft.smsc.admin.console.commands.emailsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.utli.Proxy;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;

public class EmailSmeContext extends Proxy {
    public static final byte LIMITTYPE_DAY = 0;
    public static final byte LIMITTYPE_WEEK = 0;
    public static final byte LIMITTYPE_MONTH = 0;

    public static final String LIMITCHAR_DAY_CHAR = "d";
    public static final String LIMITCHAR_DAY_WEEK = "w";
    public static final String LIMITCHAR_DAY_MONTH = "m";

    public static String EMAILSME_ID = "emailsme";
    private static final int STATUS_OK = 1;

    private static final byte COMMAND_UPDATE = 1;
    private static final byte COMMAND_DELETE = 2;
    protected static Object instanceLock = new Object();
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
    }

    public void add(byte ton, byte npi, String address, String userName, String forwardEmail,
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
            for (int i = 0; i < len; i++) {
                addr[i] = (byte) address.charAt(i);
            }
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

            int result = (int) Message.readUInt32(in);
            if (result != STATUS_OK) throw new AdminException("Could not add/update: emailsme returned error");
        }
        catch (java.io.IOException e) {
            throw new AdminException("Could not add/update: IOException");
        }
    }

    public void delete(byte ton, byte npi, String address) throws AdminException {
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
            int result = in.read();
            if (result != STATUS_OK) throw new AdminException("Could not delete: emailsme returned error");
        }
        catch (java.io.IOException e) {
            throw new AdminException("Could not delete: IOException");
        }
    }

    private void checkConnect() throws AdminException {
        disconnect();
       if (getStatus() != StatusConnected) connect(host, port);
    }
}
