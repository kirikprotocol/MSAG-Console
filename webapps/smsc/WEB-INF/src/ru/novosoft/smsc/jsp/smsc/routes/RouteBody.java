package ru.novosoft.smsc.jsp.smsc.routes;

import ru.novosoft.smsc.jsp.smsc.SmscBean;

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
	protected String[] checkedSources = null;
	protected String[] srcMasks = null;
	protected String[] checkedDestinations = null;
	protected String[] dstMasks = null;
	protected String dst_mask_sme_ = null;
	protected Set checkedSourcesSet = null;
	protected Set checkedDestinationsSet = null;
	protected Map selectedSmes = new HashMap();
	protected Map selectedMaskSmes = new HashMap();

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
		return smeManager.getSmeNames();
	}

	public boolean isSmeSelected(String dstName, String smeId)
	{
		return smeId.equals(selectedSmes.get(dstName));
	}

	public boolean isMaskSmeSelected(String dstMask, String smeId)
	{
		return smeId.equals(selectedMaskSmes.get(dstMask));
	}

	/*************************** properties *********************************/

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

	public int getPriority()
	{
		return priority;
	}

	public void setPriority(String priority)
	{
		try
		{
			this.priority = Integer.decode(priority).intValue();
		}
		catch (NumberFormatException e)
		{
			this.priority = 0;
		}
	}

	public int getServiceId()
	{
		return serviceId;
	}

	public void setServiceId(String serviceId)
	{
		try
		{
			this.serviceId = Integer.decode(serviceId).intValue();
		}
		catch (NumberFormatException e)
		{
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
}
