package ru.novosoft.smsc.admin.service.resource_group;

import java.util.HashMap;
import java.util.Map;

/**
 * TODO Удалить!
 */
public class ResourceGroupConstants {

    public static final String RESOURCEGROUP_LIBNAME_HA = "NativeResourceGroupHA";

    public static final int RESOURCEGROUP_TYPE_UNKNOWN = 0;
    public static final int RESOURCEGROUP_TYPE_SINGLE = 1;
    public static final int RESOURCEGROUP_TYPE_HS = 2;
    public static final int RESOURCEGROUP_TYPE_HA = 3;

    public static final String RESOURCEGROUP_TYPE_STRING_SINGLE = "single";
    public static final String RESOURCEGROUP_TYPE_STRING_HS = "HS";
    public static final String RESOURCEGROUP_TYPE_STRING_HA = "HA";

    public static final String RESOURCEGROUP_INSTALLTYPE_PARAM_NAME = "installation.type";
    public static final String RESOURCEGROUP_MIRRORPATH_PARAM_NAME = "installation.mirrorpath";
    public static final String RESOURCEGROUP_RG_MAP_FILE = "installation.rgmapFile";

    public static final Map SMSC_serv_IDs = new HashMap();

    static {
        SMSC_serv_IDs.put(new Byte((byte) 1), "SMSCservice1");
        SMSC_serv_IDs.put(new Byte((byte) 2), "SMSCservice2");
    }

    public static byte getTypeFromString(String sType) {
        if (sType.equals(ResourceGroupConstants.RESOURCEGROUP_TYPE_STRING_SINGLE))
            return ResourceGroupConstants.RESOURCEGROUP_TYPE_SINGLE;
        if (sType.equals(ResourceGroupConstants.RESOURCEGROUP_TYPE_STRING_HS))
            return ResourceGroupConstants.RESOURCEGROUP_TYPE_HS;
        if (sType.equals(ResourceGroupConstants.RESOURCEGROUP_TYPE_STRING_HA))
            return ResourceGroupConstants.RESOURCEGROUP_TYPE_HA;
        return ResourceGroupConstants.RESOURCEGROUP_TYPE_UNKNOWN;
    }
}
