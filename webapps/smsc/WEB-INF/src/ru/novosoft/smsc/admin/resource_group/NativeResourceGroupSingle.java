package ru.novosoft.smsc.admin.resource_group;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 22.11.2005
 * Time: 14:28:38
 * To change this template use File | Settings | File Templates.
 */
public class NativeResourceGroupSingle
{
	public final static void LoadLibrary() throws AdminException
	{
		try
		{
			System.loadLibrary(ResourceGroupConstants.RESOURCEGROUP_LIBNAME_SINGLE);
		}
		catch (UnsatisfiedLinkError e)
		{
			throw new AdminException("JNI Library is not found");
		}
	}

	public final static native long new_ResourceGroup(String jarg1);
	public final static native String[] ResourceGroup_listGroups();
	public final static native String[] ResourceGroup_listNodes(long jarg1);
	public final static native boolean ResourceGroup_onlineStatus(long jarg1, String jarg2);
	public final static native void ResourceGroup_online(long jarg1);
	public final static native void ResourceGroup_offline(long jarg1);
	public final static native void ResourceGroup_switchOver(long jarg1, String jarg2);
	public final static native void delete_ResourceGroup(long jarg1);
}
