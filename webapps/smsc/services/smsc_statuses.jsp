<%@ page import="ru.novosoft.smsc.admin.Constants,
                 ru.novosoft.smsc.admin.resource_group.ResourceGroupConstants,
                 ru.novosoft.smsc.admin.service.ServiceInfo,
                 ru.novosoft.smsc.admin.smsc_service.SmscList" %><%@
    include file="/WEB-INF/inc/localization.jsp" %><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses" /><jsp:setProperty name="bean" property="*" /><%

  String CPATH = request.getContextPath() + "/smsc";

  bean.process(request);

  byte   status = ServiceInfo.STATUS_UNKNOWN;
  String result = Constants.SMSC_SME_ID + "\r\n";

  for (byte i = 0; i < ResourceGroupConstants.SMSC_serv_IDs.size(); i++) {
    try {
      status = bean.getAppContext().getHostsManager().getServiceInfo((String)ResourceGroupConstants.SMSC_serv_IDs.get(new
              Byte(i))).getStatus();
    } catch (Throwable e) {}

    switch (status) {
      case ServiceInfo.STATUS_OFFLINE :
        result += Constants.SMSC_SME_ID + " " + getLocString("grammatic.at") + " " + SmscList.getNodeFromId(i) + " " +
                getLocString("common.statuses.offline");
        break;
      case ServiceInfo.STATUS_ONLINE1 :
      case ServiceInfo.STATUS_ONLINE2 :
        result += Constants.SMSC_SME_ID + " " + getLocString("grammatic.at") + " " + SmscList.getNodeFromId(i) + " " +
                getLocString("common.statuses.online");
        result += " " + bean.smscServStatusString(Constants.SMSC_SME_ID, i);
        break;
      case ServiceInfo.STATUS_ONLINE :
        result = result + Constants.SMSC_SME_ID + getLocString("grammatic.is") + getLocString("common.statuses.online");
      default :
        result += getLocString("common.statuses.unknown");
        break;
    }

    result += "&nbsp;&nbsp;&nbsp;";
  }

  out.print(result);%>
