<%@ page import="ru.sibinco.lib.backend.service.ServiceInfo"
 %><jsp:useBean id="bean" class="ru.sibinco.scag.beans.gw.status.Status"
 />status
<%
  bean.process(request, response);
  switch(bean.getGwStatus())
  {
    case ServiceInfo.STATUS_RUNNING:
      out.print("<img src='/images/ic_running.gif' width=10 height=10 title='running'> running");
      break;
    case ServiceInfo.STATUS_STOPPING:
      out.print("<img src='/images/ic_stopping.gif' width=10 height=10 title='stopping'> stopping");
      break;
    case ServiceInfo.STATUS_STOPPED:
      out.print("<img src='/images/ic_stopped.gif' width=10 height=10 title='stopped'> stopped");
      break;
    case ServiceInfo.STATUS_STARTING:
      out.print("<img src='/images/ic_starting.gif' width=10 height=10 title='starting'> starting");
      break;
    default:
      out.print("<img src='/images/ic_unknown.gif' width=10 height=10 title='unknown'> unknown");
      break;
  }
%>