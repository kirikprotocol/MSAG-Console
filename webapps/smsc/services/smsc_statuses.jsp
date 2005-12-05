<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo,
                ru.novosoft.smsc.admin.Constants"%><%@
 include file="/WEB-INF/inc/localization.jsp"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"/><jsp:setProperty name="bean" property="*"/><%
String CPATH = request.getContextPath() + "/smsc";
bean.process(request);
byte status = ServiceInfo.STATUS_UNKNOWN;
    try
    {
		status = bean.getAppContext().getHostsManager().getServiceInfo(Constants.SMSC_SME_ID).getStatus();
	}
    catch (Throwable e)
    {
        errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR), e);
    }
String result = Constants.SMSC_SME_ID + "\r\n";
    for (Iterator i = Constants.SMSC_serv_IDs.keySet().iterator(); i.hasNext();)
    {
        Byte id = (Byte) i.next();
		switch (status)
		{
			case ServiceInfo.STATUS_OFFLINE:
                result = result + Constants.SMSC_SME_ID + id + " " + getLocString("grammatic.is") + " ";
				result = result + getLocString("common.statuses.offline") + "_";
                result += bean.smscServStatusString((String) Constants.SMSC_serv_IDs.get(id));
				break;
			case ServiceInfo.STATUS_ONLINE1:
                result = result + Constants.SMSC_SME_ID + id + " " + getLocString("grammatic.is") + " ";
                if (id.equals(new Byte(ServiceInfo.STATUS_ONLINE1)))
				    result = result + getLocString("common.statuses.online") + "_";
				    else result = result + getLocString("common.statuses.offline") + "_";
                result += bean.smscServStatusString((String) Constants.SMSC_serv_IDs.get(id));
				break;
			case ServiceInfo.STATUS_ONLINE2:
                result = result + Constants.SMSC_SME_ID + id + " " + getLocString("grammatic.is") + " ";
                if (id.equals(new Byte(ServiceInfo.STATUS_ONLINE2)))
				    result = result + getLocString("common.statuses.online") + "_";
				    else result = result + getLocString("common.statuses.offline") + "_";
                result += bean.smscServStatusString((String) Constants.SMSC_serv_IDs.get(id));
				break;
			default:
				result += getLocString("common.statuses.unknown");
				break;
		}
        result += " ";
    }
out.print(result);%>