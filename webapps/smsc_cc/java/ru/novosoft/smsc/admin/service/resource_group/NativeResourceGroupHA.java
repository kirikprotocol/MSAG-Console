package ru.novosoft.smsc.admin.service.resource_group;

import ru.novosoft.smsc.admin.AdminException;

public final class NativeResourceGroupHA {

  public final static void LoadLibrary() throws AdminException {
    try {
      System.loadLibrary("NativeResourceGroupHA");
    } catch (UnsatisfiedLinkError e) {
      e.printStackTrace();
      throw new ResourceGroupException("jni_lib_not_found");
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
