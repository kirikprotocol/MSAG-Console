package ru.novosoft.smsc.jsp.smsc.routes;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.util.SortedList;

import java.util.*;


/**
 * Created by igork
 * Date: Feb 3, 2003
 * Time: 2:48:46 PM
 */
public class RouteBody extends SmscBean
{
  protected String mbSave = null;
  protected String mbCancel = null;
  protected String routeId = null;
  protected int priority = 0;
  protected boolean permissible = false;
  protected boolean billing = false;
  protected boolean archiving = false;
  protected boolean suppressDeliveryReports = false;
  protected boolean active = false;
  protected int serviceId = 0;
  protected String srcSmeId = null;
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
  protected boolean forceReplayPath = false;
  protected String notes = "";
  protected boolean forceDelivery = false;
  protected long aclId = -1;

  public boolean isSrcChecked(String srcName)
  {
    return checkedSourcesSet.contains(srcName);
  }

  public boolean isDstChecked(String dstName)
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

  public boolean isSmeSelected(String dstName, String smeId)
  {
    return smeId.equals(selectedSmes.get(dstName));
  }

  public boolean isMaskSmeSelected(String dstMask, String smeId)
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

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getRouteId()
  {
    return routeId;
  }

  public void setRouteId(String routeId)
  {
    this.routeId = routeId;
  }

  public boolean isPermissible()
  {
    return permissible;
  }

  public void setPermissible(boolean permissible)
  {
    this.permissible = permissible;
  }

  public boolean isBilling()
  {
    return billing;
  }

  public void setBilling(boolean billing)
  {
    this.billing = billing;
  }

  public boolean isArchiving()
  {
    return archiving;
  }

  public void setArchiving(boolean archiving)
  {
    this.archiving = archiving;
  }

  public String[] getCheckedSources()
  {
    return checkedSources;
  }

  public void setCheckedSources(String[] checkedSources)
  {
    this.checkedSources = checkedSources;
  }

  public String[] getSrcMasks()
  {
    return srcMasks;
  }

  public void setSrcMasks(String[] srcMasks)
  {
    this.srcMasks = srcMasks;
  }

  public String[] getCheckedDestinations()
  {
    return checkedDestinations;
  }

  public void setCheckedDestinations(String[] checkedDestinations)
  {
    this.checkedDestinations = checkedDestinations;
  }

  public String[] getDstMasks()
  {
    return dstMasks;
  }

  public void setDstMasks(String[] dstMasks)
  {
    this.dstMasks = dstMasks;
  }

  public String getDst_mask_sme_()
  {
    return dst_mask_sme_;
  }

  public void setDst_mask_sme_(String dst_mask_sme_)
  {
    this.dst_mask_sme_ = dst_mask_sme_;
  }

  public String getPriority()
  {
    return Integer.toString(priority);
  }

  public void setPriority(String priority)
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

  public void setServiceId(String serviceId)
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

  public void setSuppressDeliveryReports(boolean suppressDeliveryReports)
  {
    this.suppressDeliveryReports = suppressDeliveryReports;
  }

  public boolean isActive()
  {
    return active;
  }

  public void setActive(boolean active)
  {
    this.active = active;
  }

  public String getSrcSmeId()
  {
    return srcSmeId;
  }

  public void setSrcSmeId(String srcSmeId)
  {
    this.srcSmeId = srcSmeId;
  }

  public String getDefaultSubjectSme(String subjId)
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

  public void setDeliveryMode(String deliveryMode)
  {
    this.deliveryMode = deliveryMode;
  }

  public String getForwardTo()
  {
    return forwardTo;
  }

  public void setForwardTo(String forwardTo)
  {
    this.forwardTo = forwardTo;
  }

  public boolean isHide()
  {
    return hide;
  }

  public void setHide(boolean hide)
  {
    this.hide = hide;
  }

  public boolean isForceReplayPath()
  {
    return forceReplayPath;
  }

  public void setForceReplayPath(boolean forceReplayPath)
  {
    this.forceReplayPath = forceReplayPath;
  }

  public String getNotes()
  {
    return notes;
  }

  public void setNotes(String notes)
  {
    this.notes = notes;
  }

  public boolean isForceDelivery()
  {
    return forceDelivery;
  }

  public void setForceDelivery(boolean forceDelivery)
  {
    this.forceDelivery = forceDelivery;
  }

  public long getAclId()
  {
    return aclId;
  }

  public void setAclId(long aclId)
  {
    this.aclId = aclId;
  }
}
