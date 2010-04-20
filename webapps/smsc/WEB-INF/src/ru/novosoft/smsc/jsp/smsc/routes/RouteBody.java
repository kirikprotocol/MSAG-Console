package ru.novosoft.smsc.jsp.smsc.routes;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.util.SortedList;

import java.util.*;


/**
 * Created by igork Date: Feb 3, 2003 Time: 2:48:46 PM
 */
public class RouteBody extends SmscBean
{
  protected String mbSave = null;
  protected String mbCancel = null;
  protected String routeId = null;
  protected int priority = 0;
  protected boolean permissible = false;
  protected byte billing = Route.BILLING_FALSE;
  protected boolean transit = false;
  protected boolean archiving = false;
  protected boolean suppressDeliveryReports = false;
  protected boolean active = false;
  protected int serviceId = 0;
  protected String srcSmeId = null;
  protected String backupSmeId = null;

  protected String[] checkedSources = null;
  protected String[] srcMasks = null;
  protected String[] checkedDestinations = null;
  protected String[] dstMasks = null;
  protected String dst_mask_sme_ = null;
  protected Set checkedSourcesSet = null;
  protected Set checkedDestinationsSet = null;
  protected Map selectedSmes = new HashMap();
  protected Map selectedMaskSmes = new HashMap();
  protected String deliveryMode = null;
  protected String forwardTo = null;
  protected boolean hide = false;
  protected byte replayPath = Route.REPLAY_PATH_PASS;
  protected String notes = "";
  protected boolean forceDelivery = false;
  protected long aclId = -1;
  protected boolean allowBlocked = false;
  protected long providerId = -1;
  protected long categoryId = -1;
  protected String providerIdStr = null;
  protected String categoryIdStr = null;
  protected ProviderManager providerManager = null;
  protected CategoryManager categoryManager = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;
    providerManager = appContext.getProviderManager();
    categoryManager = appContext.getCategoryManager();
    return result;
  }

  public boolean isSrcChecked(final String srcName)
  {
    return checkedSourcesSet.contains(srcName);
  }

  public boolean isDstChecked(final String dstName)
  {
    return checkedDestinationsSet.contains(dstName);
  }

  public Collection getAllSubjects()
  {
    return routeSubjectManager.getSubjects().getNames();
  }

  public Collection getAllSmes()
  {
    return new SortedList(smeManager.getSmeNames());
  }

  public Collection getProviders()
  {
    return new SortedList(providerManager.getProviders().values());
  }

  public Collection getCategories()
  {
    return new SortedList(categoryManager.getCategories().values());
  }

  public boolean isSmeSelected(final String dstName, final String smeId)
  {
    return smeId.equals(selectedSmes.get(dstName));
  }

  public boolean isMaskSmeSelected(final String dstMask, final String smeId)
  {
    return smeId.equals(selectedMaskSmes.get(dstMask));
  }

  public List getAclNames()
  {
    try {
      return appContext.getAclManager().getAclNames();
    } catch (AdminException e) {
      logger.error("Could not get list of ACL names", e);
      return new LinkedList();
    }
  }

  /**
   * ************************ properties ********************************
   */

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(final String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(final String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getRouteId()
  {
    return routeId;
  }

  public void setRouteId(final String routeId)
  {
    this.routeId = routeId;
  }

  public boolean isPermissible()
  {
    return permissible;
  }

  public void setPermissible(final boolean permissible)
  {
    this.permissible = permissible;
  }

  public byte getBilling()
  {
    return billing;
  }

  public void setBilling(final byte billing)
  {
    this.billing = billing;
  }

  public boolean isTransit()
  {
    return transit;
  }

  public void setTransit(boolean transit)
  {
    this.transit = transit;
  }

  public boolean isArchiving()
  {
    return archiving;
  }

  public void setArchiving(final boolean archiving)
  {
    this.archiving = archiving;
  }

  public String[] getCheckedSources()
  {
    return checkedSources;
  }

  public void setCheckedSources(final String[] checkedSources)
  {
    this.checkedSources = checkedSources;
  }

  public String[] getSrcMasks()
  {
    return srcMasks;
  }

  public void setSrcMasks(final String[] srcMasks)
  {
    this.srcMasks = srcMasks;
  }

  public String[] getCheckedDestinations()
  {
    return checkedDestinations;
  }

  public void setCheckedDestinations(final String[] checkedDestinations)
  {
    this.checkedDestinations = checkedDestinations;
  }

  public String[] getDstMasks()
  {
    return dstMasks;
  }

  public void setDstMasks(final String[] dstMasks)
  {
    this.dstMasks = dstMasks;
  }

  public String getDst_mask_sme_()
  {
    return dst_mask_sme_;
  }

  public void setDst_mask_sme_(final String dst_mask_sme_)
  {
    this.dst_mask_sme_ = dst_mask_sme_;
  }

  public String getPriority()
  {
    return Integer.toString(priority);
  }

  public void setPriority(final String priority)
  {
    try {
      this.priority = Integer.decode(priority).intValue();
    } catch (NumberFormatException e) {
      this.priority = 0;
    }
  }

  public String getServiceId()
  {
    return Integer.toString(serviceId);
  }

  public void setServiceId(final String serviceId)
  {
    try {
      this.serviceId = Integer.decode(serviceId).intValue();
    } catch (NumberFormatException e) {
      this.serviceId = 0;
    }
  }

  public boolean isSuppressDeliveryReports()
  {
    return suppressDeliveryReports;
  }

  public void setSuppressDeliveryReports(final boolean suppressDeliveryReports)
  {
    this.suppressDeliveryReports = suppressDeliveryReports;
  }

  public boolean isActive()
  {
    return active;
  }

  public void setActive(final boolean active)
  {
    this.active = active;
  }

  public String getSrcSmeId()
  {
    return srcSmeId;
  }

  public void setSrcSmeId(final String srcSmeId)
  {
    this.srcSmeId = srcSmeId;
  }

  public String getBackupSmeId() 
  {
    return backupSmeId;
  }

  public void setBackupSmeId(String backupSmeId)
  {
    this.backupSmeId = backupSmeId;
  }

  public String getProviderIdStr()
  {
    return providerIdStr;
  }

  public void setProviderIdStr(String providerIdStr)
  {
    this.providerIdStr = providerIdStr;
  }

  public String getCategoryIdStr()
  {
    return categoryIdStr;
  }

  public void setCategoryIdStr(String categoryIdStr)
  {
    this.categoryIdStr = categoryIdStr;
  }

  public String getDefaultSubjectSme(final String subjId)
  {
    try {
      return routeSubjectManager.getSubjects().get(subjId).getDefaultSme().getId();
    } catch (Throwable e) {
      logger.error("Could not get default SME for subject \"" + subjId + "\"", e);
      return "";
    }
  }

  public String getDeliveryMode()
  {
    return deliveryMode;
  }

  public void setDeliveryMode(final String deliveryMode)
  {
    this.deliveryMode = deliveryMode;
  }

  public String getForwardTo()
  {
    return forwardTo;
  }

  public void setForwardTo(final String forwardTo)
  {
    this.forwardTo = forwardTo;
  }

  public boolean isHide()
  {
    return hide;
  }

  public void setHide(final boolean hide)
  {
    this.hide = hide;
  }

  public byte getReplayPath()
  {
    return replayPath;
  }

  public void setReplayPath(final byte replayPath)
  {
    this.replayPath = replayPath;
  }

  public String getNotes()
  {
    return notes;
  }

  public void setNotes(final String notes)
  {
    this.notes = notes;
  }

  public boolean isForceDelivery()
  {
    return forceDelivery;
  }

  public void setForceDelivery(final boolean forceDelivery)
  {
    this.forceDelivery = forceDelivery;
  }

  public long getAclId()
  {
    return aclId;
  }

  public void setAclId(final long aclId)
  {
    this.aclId = aclId;
  }

  public boolean isAllowBlocked()
  {
    return allowBlocked;
  }

  public void setAllowBlocked(final boolean allowBlocked)
  {
    this.allowBlocked = allowBlocked;
  }

  public long getProviderId()
  {
    return providerId;
  }

  public void setProviderId(final long providerId)
  {
    this.providerId = providerId;
  }

  public long getCategoryId()
  {
    return categoryId;
  }

  public void setCategoryId(final long categoryId)
  {
    this.categoryId = categoryId;
  }
}
