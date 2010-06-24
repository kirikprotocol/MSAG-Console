package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.resource_group.ResourceGroupConstants;

import java.io.File;
import java.io.IOException;

public class Constants {
    // Note: bitmasks 1,2,4,8,...
    public static final int INST_MODE_SMSC=1;
    public static final int INST_MODE_INFO_SME=2;

    public static final int ROUTE_ID_MAXLENGTH = 32;
    public static final int CLOSED_GROUP_NAME_MAXLENGTH = 64;

    public static final String SMSC_SME_ID = "SMSC";

    public static final String TomcatRealmName = "SMSC.SmscRealm";
    public static final long ServicesRefreshTimeoutMillis = 1000;

    public static final int MAX_PRIORITY = 0x7FFF;
    public static final String SMSC_ERROR_MESSAGES_ATTRIBUTE_NAME = "SMSC_ERROR_MESSAGES_ATTRIBUTE";

    public static final String ARCHIVE_DAEMON_SVC_ID = "ArchiveDaemon";

    public static final String CONSOLE_SESSION_ID = "Console Session";
    public static final long Day = 86400000;
    public static final long Hour = 3600000;

    public static int instType = ResourceGroupConstants.RESOURCEGROUP_TYPE_SINGLE;
    public static String HSMODE_MIRRORFILES_PATH = null;

    public static int instMode = INST_MODE_SMSC;

    private static boolean mirrorSaveErrorAppeared = false;

    public static boolean isMirrorNeeded() {
      return (instType == ResourceGroupConstants.RESOURCEGROUP_TYPE_HS) && !mirrorSaveErrorAppeared;
//        if (instType == ResourceGroupConstants.RESOURCEGROUP_TYPE_HS)
//            return true;
//        else
//            return false;
    }

    public static File getMirrorFile(File file) throws IOException {
        File result = new File(HSMODE_MIRRORFILES_PATH + file.getAbsolutePath());
        if (!result.exists()) {
            result.getParentFile().mkdirs();
            result.createNewFile();
        }
        return result;
    }

    public static void setMirrorSaveErrorAppeared(boolean value) {
      mirrorSaveErrorAppeared = value;
    }
}
