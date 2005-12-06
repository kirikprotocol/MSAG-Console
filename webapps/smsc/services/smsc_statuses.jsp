<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo,
                ru.novosoft.smsc.admin.Constants,
                ru.novosoft.smsc.admin.resource_group.ResourceGroupConstants"%><%@
 include file="/WEB-INF/inc/localization.jsp"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"/><jsp:setProperty name="bean" property="*"/><%
String CPATH = request.getContextPath() + "/smsc";
bean.process(request);
byte status = ServiceInfo.STATUS_UNKNOWN;
    String result = Constants.SMSC_SME_ID + "\r\n";
    for (Iterator i = ResourceGroupConstants.SMSC_serv_IDs.keySet().iterator(); i.hasNext();)
    {
        Byte id = (Byte) i.next();
        try
        {
		    status = bean.getAppContext().getHostsManager().getServiceInfo((String) ResourceGroupConstants.SMSC_serv_IDs.get(id)).getStatus();
	    }
        catch (Throwable e) {}
		switch (status)
		{
			case ServiceInfo.STATUS_OFFLINE:
                result = result + Constants.SMSC_SME_ID + id + " " + getLocString("grammatic.is") + " ";
				result = result + getLocString("common.statuses.offline") + "_";
                result += bean.smscServStatusString(Constants.SMSC_SME_ID, id.byteValue());
				break;
			case ServiceInfo.STATUS_ONLINE1:
                result = result + Constants.SMSC_SME_ID + id + " " + getLocString("grammatic.is") + " ";
                if (id.equals(new Byte(ServiceInfo.STATUS_ONLINE1)))
				    result = result + getLocString("common.statuses.online") + "_";
				    else result = result + getLocString("common.statuses.offline") + "_";
                result += bean.smscServStatusString(Constants.SMSC_SME_ID, id.byteValue());
				break;
			case ServiceInfo.STATUS_ONLINE2:
                result = result + Constants.SMSC_SME_ID + id + " " + getLocString("grammatic.is") + " ";
                if (id.equals(new Byte(ServiceInfo.STATUS_ONLINE2)))
				    result = result + getLocString("common.statuses.online") + "_";
				    else result = result + getLocString("common.statuses.offline") + "_";
                result += bean.smscServStatusString(Constants.SMSC_SME_ID, id.byteValue());
				break;
			default:
				result += getLocString("common.statuses.unknown");
				break;
		}
        result += " ";
    }
out.print(result);%>