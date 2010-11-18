package mobi.eyeline.informer.admin.service.resource_group;

import java.io.File;


class ResourceGroupHAImpl extends ResourceGroupImpl {

  protected final long swigCPtr;
  protected final boolean swigCMemOwn;
  private final File baseDir;

  protected ResourceGroupHAImpl(String name, File baseDir, long cPtr, boolean cMemoryOwn) {
    super(name);
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
    this.baseDir = baseDir;
  }

  public ResourceGroupHAImpl(String id, String name, File baseDir) {
    this(name, baseDir, NativeResourceGroupHA.new_ResourceGroup(id), true);
    if (logger.isInfoEnabled())
      logger.info("Resource group (haname=" + id + ", smename=" + name + ") created");
  }

  public void switchOver(String nodeName) {
    NativeResourceGroupHA.ResourceGroup_switchOver(swigCPtr, nodeName);
  }

  public String[] listNodes() {
    return NativeResourceGroupHA.ResourceGroup_listNodes(swigCPtr);
  }

  public File getBaseDir() {
    return baseDir;
  }

  public String getOnlineStatus() {
    String[] rgNodes = listNodes();
    for (String rgNode : rgNodes) {
      if (NativeResourceGroupHA.ResourceGroup_onlineStatus(swigCPtr, rgNode))
        return rgNode;
    }
    return null;
  }

  public void offline() {
    NativeResourceGroupHA.ResourceGroup_offline(swigCPtr);
  }

  public void online() {
    NativeResourceGroupHA.ResourceGroup_online(swigCPtr);
  }

  public void delete() {
    NativeResourceGroupHA.delete_ResourceGroup(swigCPtr);
  }
}
