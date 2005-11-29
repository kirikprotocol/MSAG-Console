package ru.novosoft.smsc.admin.resource_group;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 06.09.2005
 * Time: 15:20:20
 * To change this template use File | Settings | File Templates.
 */
public class ResourceGroupConstants
{
	public static final String RESOURCEGROUP_LIBNAME_SINGLE = "NativeResourceGroupSingle";
	public static final String RESOURCEGROUP_LIBNAME_HS = "NativeResourceGroupHS";
	public static final String RESOURCEGROUP_LIBNAME_HA = "NativeResourceGroupHA";

	public static final int RESOURCEGROUP_TYPE_UNKNOWN = 0;
	public static final int RESOURCEGROUP_TYPE_SINGLE = 1;
	public static final int RESOURCEGROUP_TYPE_HS = 2;
	public static final int RESOURCEGROUP_TYPE_HA = 3;

	public static final String RESOURCEGROUP_TYPE_STRING_SINGLE = "single";
	public static final String RESOURCEGROUP_TYPE_STRING_HS = "HS";
	public static final String RESOURCEGROUP_TYPE_STRING_HA = "HA";

	public static final String RESOURCEGROUP_INSTALLTYPE_PARAM_NAME = "installation.type";

	public static byte getTypeFromString(String sType)
	{
		if (sType.equals(ResourceGroupConstants.RESOURCEGROUP_TYPE_STRING_SINGLE))
			return ResourceGroupConstants.RESOURCEGROUP_TYPE_SINGLE;
		if (sType.equals(ResourceGroupConstants.RESOURCEGROUP_TYPE_STRING_HS))
			return ResourceGroupConstants.RESOURCEGROUP_TYPE_HS;
		if (sType.equals(ResourceGroupConstants.RESOURCEGROUP_TYPE_STRING_HA))
			return ResourceGroupConstants.RESOURCEGROUP_TYPE_HA;
		return ResourceGroupConstants.RESOURCEGROUP_TYPE_UNKNOWN;
	}
}
