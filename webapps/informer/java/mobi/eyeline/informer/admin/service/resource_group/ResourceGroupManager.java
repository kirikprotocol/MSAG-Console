package mobi.eyeline.informer.admin.service.resource_group;

import mobi.eyeline.informer.admin.AdminException;
import org.apache.log4j.Category;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.Properties;

/**
 * Класс - управляющий ресурс-группами
 */
public class ResourceGroupManager {

  private static final Category logger = Category.getInstance(ResourceGroupManager.class);

  private final ResourceGroupList resGroups = new ResourceGroupList();
  private final Properties resGroupsNames = new Properties();
  private final File servicesDir;

  public ResourceGroupManager(File resourceGroupsFile, File servicesDir) throws AdminException {

    this.servicesDir = servicesDir;

    if (logger.isDebugEnabled())
      logger.debug("Initializing resource group manager...");

    InputStream is = null;
    try {
      is = new FileInputStream(resourceGroupsFile);
      resGroupsNames.load(is);
    } catch (Exception e) {
      logger.warn("Could not init RG mapping", e);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }
    }

    NativeResourceGroupHA.LoadLibrary();
    if (logger.isDebugEnabled())
      logger.debug("JNI Library loaded.");
    refreshHAResGroupList();


    if (logger.isDebugEnabled())
      logger.debug("Resource group manager initialized.");
  }


//  public void add(ResourceGroup rg) throws AdminException {
//    resGroups.add(rg);
//  }

  public ResourceGroup get(String name) throws AdminException {
    return resGroups.get(name);
  }

//  public ResourceGroup remove(String name) throws AdminException {
//    final ResourceGroup rg = get(name);
//    resGroups.remove(name);
//    return rg;
//  }

  public List<String> getResourceGroupsNames() {
    return resGroups.getResourceGroupsNames();
  }

  public int getResourceGroupsCount() {
    return resGroups.getResourceGroupsNames().size();
  }

  public boolean contains(String name) {
    return resGroups.contains(name);
  }

  public void refreshResGroupList() throws AdminException {
    refreshHAResGroupList();
  }

  private void refreshHAResGroupList() {

    if (logger.isDebugEnabled())
      logger.debug("ResourceGroup_listGroups calling");

    String[] rgIds = NativeResourceGroupHA.ResourceGroup_listGroups();

    if (logger.isDebugEnabled())
      logger.debug("ResourceGroup_listGroups returned resource groups:" + rgIds.length);

    for (String rgId : rgIds) {
      try {
        String name = resGroupsNames.getProperty(rgId);
        if (name == null)
          throw new ResourceGroupException("unknown_resource_group", rgId);
        resGroups.add(new ResourceGroupHAImpl(rgId, name, new File(servicesDir, name)));
        if (logger.isDebugEnabled())
          logger.debug("Resource group \"" + name + "\" added");
      } catch (AdminException e) {
        logger.error("Couldn't init resource group:" + rgId, e);
      }
    }
  }
}
