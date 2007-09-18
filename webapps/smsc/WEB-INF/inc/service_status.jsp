<%@ page import="ru.novosoft.smsc.admin.Constants,
                 ru.novosoft.smsc.admin.resource_group.ResourceGroupConstants,
                 ru.novosoft.smsc.admin.service.ServiceInfo,
                 ru.novosoft.smsc.admin.smsc_service.SmscList,
                 ru.novosoft.smsc.jsp.SMSCAppContext,
                 ru.novosoft.smsc.util.StringEncoderDecoder" %>
<%@ include file="/WEB-INF/inc/show_sme_status.jsp" %>
<script>
  var serviceStatusWaiters = new Array();
  var smeStatusWaiters = new Array();
  var smscStatusWaiters = new Array();

  var StatusWaiter = new Class({
    options: {
      elementId : 'id',
      serviceId : 'id',
      html : false
    },

    initialize : function(options) {
      this.setOptions(options);
    }
  });

  StatusWaiter.implement(new Options);

  function registerServiceStatusWaiter(statusWaiter) {
    serviceStatusWaiters[serviceStatusWaiters.length] = statusWaiter;
  }
  function registerSmeStatusWaiter(statusWaiter) {
    smeStatusWaiters[smeStatusWaiters.length] = statusWaiter;
  }
  function registerSmscStatusWaiter(statusWaiter) {
    smscStatusWaiters[smscStatusWaiters.length] = statusWaiter;
  }
</script>
<%!
    String smeStatus(SMSCAppContext appContext, String serviceId) {
        SmeStatus status = null;

        try {
            status = appContext.getSmeManager().smeStatus(serviceId);
        } catch (Throwable e) {
        }

        String elem_id = "CONNECTION_STATUSERVICE_" + StringEncoderDecoder.encode(serviceId);

        return "<span id=\"" + elem_id + "\" datasrc=#tdcConnStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode
                (serviceId) + "\" " + showSmeStatus(status) + "\n" + addSmeStatusWaiterScript(elem_id, "\"" + serviceId + "\"");
    }

    String serviceStatus(SMSCAppContext appContext, String serviceId) {
        String elem_id = "RUNNING_STATUSERVICE_" + StringEncoderDecoder.encode(serviceId);

        return serviceStatus(appContext, serviceId, elem_id);
    }

    String serviceStatus(SMSCAppContext appContext, String serviceId, String elem_id) {
        byte status = ServiceInfo.STATUS_UNKNOWN;

        try {
            status = appContext.getHostsManager().getServiceInfo(serviceId).getStatus();
        } catch (Throwable e) {
        }

        String result = "<span id=\"" + elem_id + "\" datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" +
                StringEncoderDecoder.encode(serviceId) + "\">";

        switch (status) {
            case ServiceInfo.STATUS_OFFLINE :
                //            result += "<img src=\"/images/ic_offline.png\" width=16 height=16 title='offline'>";
                result += getLocString("common.statuses.offline");
                break;
            case ServiceInfo.STATUS_ONLINE1 :
            case ServiceInfo.STATUS_ONLINE2 :
                //            result += "<img src=\"/images/ic_online1.png\" width=16 height=16 title='online'>";
                result += getLocString("common.statuses.onlineat") + " " + SmscList.getNodeFromId(status);
                break;
            case ServiceInfo.STATUS_ONLINE :
                //            result += "<img src=\"/images/ic_online.png\" width=16 height=16 title='online'>";
                result += getLocString("common.statuses.online");
                break;
            default :
                //            result += "<img src=\"/images/ic_unknown.png\" width=16 height=16 title='unknown'>";
                result += getLocString("common.statuses.unknown");
                break;
        }

        result += "</span>";

        result += addServiceStatusWaiterScript(elem_id, serviceId);

        return result;
    }

    String smscStatus(SMSCAppContext appContext) {
        String elem_id = "RUNNING_STATUSERVICE_" + StringEncoderDecoder.encode(Constants.SMSC_SME_ID);
        String result = "<span id=\"" + elem_id + "\" datasrc=#tdcSmscStatuses DATAFORMATAS=html datafld=\"" +
                StringEncoderDecoder.encode(Constants.SMSC_SME_ID) + "\">";

        for (byte i = 1; i <= ResourceGroupConstants.SMSC_serv_IDs.size(); i++) {
            byte status = ServiceInfo.STATUS_UNKNOWN;

            try {
                status = appContext.getHostsManager().getServiceInfo((String) ResourceGroupConstants.SMSC_serv_IDs.get(new Byte(i)))
                        .getStatus();
            } catch (Throwable e) {
            }

            switch (status) {
                case ServiceInfo.STATUS_OFFLINE :
                    result += Constants.SMSC_SME_ID + " " + getLocString("grammatic.at") + " " + SmscList.getNodeFromId(i) + " " +
                            getLocString("common.statuses.offline");
                    break;
                case ServiceInfo.STATUS_ONLINE1 :
                case ServiceInfo.STATUS_ONLINE2 :
                    result += Constants.SMSC_SME_ID + " " + getLocString("grammatic.at") + " " + SmscList.getNodeFromId(i) + " " +
                            getLocString("common.statuses.online");
                    result += " " + smscServStatusString(appContext, Constants.SMSC_SME_ID, i);
                    break;
                case ServiceInfo.STATUS_ONLINE :
                    result = result + Constants.SMSC_SME_ID + getLocString("grammatic.is") + getLocString("common.statuses.online");
                default :
                    result += getLocString("common.statuses.unknown");
                    break;
            }

            result += "&nbsp;&nbsp;&nbsp;";
        }

        result += "</span>";

        result += addSmscStatusWaiterScript(elem_id, Constants.SMSC_SME_ID);

        return result;
    }

    String smscServStatus(SMSCAppContext appContext, String serviceId, byte nodeId) {
        String elem_id = "RUNNING_STATUSERVICE_" + StringEncoderDecoder.encode(serviceId);
        String result = "<span id=\"" + elem_id + "\" datasrc=#tdcSmscStatuses DATAFORMATAS=html datafld=\"" +
                StringEncoderDecoder.encode(Byte.toString(nodeId)) + "\">";

        result += smscServStatusString(appContext, serviceId, nodeId);
        result += "</span>";

        return result;
    }

    String smscServStatusString(SMSCAppContext appContext, String serviceId, byte nodeId) {
        //the same code is in Statuses.java
        byte status = ServiceInfo.STATUS_UNKNOWN;

        try {
            status = appContext.getHostsManager().getServiceInfo(serviceId).getStatus();
        } catch (Throwable e) {
        }

        switch (status) {
            case ServiceInfo.STATUS_OFFLINE :
                return getLocString("common.statuses.deactivated");
            case ServiceInfo.STATUS_ONLINE1 :
                if (nodeId == ServiceInfo.STATUS_ONLINE1) {
                    return getLocString("common.statuses.activated");
                } else {
                    return getLocString("common.statuses.deactivated");
                }
            case ServiceInfo.STATUS_ONLINE2 :
                if (nodeId == ServiceInfo.STATUS_ONLINE2) {
                    return getLocString("common.statuses.activated");
                } else {
                    return getLocString("common.statuses.deactivated");
                }
            default :
                return getLocString("common.statuses.unknown");
        }
    }

    String getStatusWaiter(String elementId, String serviceId, boolean html) {
      return "new StatusWaiter({elementId: '" +elementId + "', serviceId: '" + serviceId + "', html: " + html + "})";
    }

    String addServiceStatusWaiterScript(String elementId, String serviceId) {
      final StringBuffer buffer = new StringBuffer();
      buffer.append("\n<script>\n");
      buffer.append("registerServiceStatusWaiter(").append(getStatusWaiter(elementId, serviceId, false)).append(");\n");
      buffer.append("</script>\n");
      return buffer.toString();
    }

    String addSmeStatusWaiterScript(String elementId, String serviceId) {
      final StringBuffer buffer = new StringBuffer();
      buffer.append("\n<script>\n");
      buffer.append("registerSmeStatusWaiter(").append(getStatusWaiter(elementId, serviceId, true)).append(");\n");
      buffer.append("</script>\n");
      return buffer.toString();
    }

    String addSmscStatusWaiterScript(String elementId, String serviceId) {
      final StringBuffer buffer = new StringBuffer();
      buffer.append("\n<script>\n");
      buffer.append("registerSmscStatusWaiter(").append(getStatusWaiter(elementId, serviceId, false)).append(");\n");
      buffer.append("</script>\n");
      return buffer.toString();
    }

%>
<%--<%--%>
<%--    if (browserType == BROWSER_TYPE_MSIE) {--%>
<%--%>--%>
<%--<OBJECT id="tdcStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">--%>
<%--    <PARAM NAME="DataURL" VALUE="<%= CPATH %>/services/statuses.jsp">--%>
<%--    <PARAM NAME="UseHeader" VALUE="True">--%>
<%--    <PARAM NAME="TextQualifier" VALUE='"'>--%>
<%--</OBJECT>--%>
<%--<OBJECT id="tdcConnStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">--%>
<%--    <PARAM NAME="DataURL" VALUE="<%= CPATH %>/services/connected_statuses.jsp">--%>
<%--    <PARAM NAME="UseHeader" VALUE="True">--%>
<%--    <PARAM NAME="TextQualifier" VALUE='"'>--%>
<%--</OBJECT>--%>
<%--<OBJECT id="tdcSmscStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">--%>
<%--    <PARAM NAME="DataURL" VALUE="<%= CPATH %>/services/smsc_statuses.jsp">--%>
<%--    <PARAM NAME="UseHeader" VALUE="True">--%>
<%--    <PARAM NAME="TextQualifier" VALUE='"'>--%>
<%--</OBJECT>--%>
<%--<script>--%>
<%--    function refreshStatus() {--%>
<%--        document.getElementById('tdcStatuses').DataURL = document.getElementById('tdcStatuses').DataURL;--%>
<%--        document.getElementById('tdcStatuses').reset();--%>
<%--        document.getElementById('tdcSmscStatuses').DataURL = document.getElementById('tdcSmscStatuses').DataURL;--%>
<%--        document.getElementById('tdcSmscStatuses').reset();--%>
<%--        document.getElementById('tdcConnStatuses').DataURL = document.getElementById('tdcConnStatuses').DataURL;--%>
<%--        document.getElementById('tdcConnStatuses').reset();--%>
<%--        window.setTimeout(refreshStatus, 10000);--%>
<%--    }--%>
<%--    refreshStatus();--%>
<%--</script>--%>
<%--<%--%>
<%--    }--%>
<%--%>--%>
<script>
  function sendAsynchronousRequest(url, responseHandler) {
    var xhr = new XHR({method: 'post'});
    xhr.addEvent('onSuccess', responseHandler);
    xhr.send(url, '');
  }
  function processResponse(text, xml, statusWaiters) {
    var txt = new String(text);
    var i = txt.indexOf("\r\n");

    if (i > 0) {
      var services = new Array();
      services = txt.substring(0, i).split(', ');
      var statuses = new Array();
      statuses = txt.substring(i, txt.length).split(', ');

      var statusWaiter;
      var element;
      for (var j = 0; j < statusWaiters.length; j++) {
        statusWaiter = statusWaiters[j];
        for (var k = 0; k < services.length; k++) {
          if (services[k] == statusWaiter.options.serviceId) {
            element = document.getElementById(statusWaiter.options.elementId);
            if (statusWaiter.options.html)
              element.innerHTML = statuses[k];
            else
              element.innerText = statuses[k];
            break;
          }
        }
      }
    }
  }

  // SMSC status
  function refreshSmscStatuses() {
    sendAsynchronousRequest('<%= CPATH %>/services/smsc_statuses.jsp', processResponseForSmscStatuses);
    window.setTimeout(refreshSmscStatuses, 10000);
  }
  function processResponseForSmscStatuses(text, xml) {
    processResponse(text, xml, smscStatusWaiters);
  }

  // SME status
  function refreshSmeStatuses() {
    sendAsynchronousRequest('<%= CPATH %>/services/connected_statuses.jsp', processResponseForSmeStatuses);
    window.setTimeout(refreshSmeStatuses, 10000);
  }
  function processResponseForSmeStatuses(text, xml) {
    processResponse(text, xml, smeStatusWaiters);
  }

  // Service status
  function refreshServiceStatuses() {
    sendAsynchronousRequest('<%= CPATH %>/services/statuses.jsp', processResponseForServiceStatuses)
    window.setTimeout(refreshServiceStatuses, 10000);
  }
  function processResponseForServiceStatuses(text, xml) {
    processResponse(text, xml, serviceStatusWaiters);
  }
</script>
