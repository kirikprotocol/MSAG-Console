<%@ page  pageEncoding="UTF-8"
          contentType="text/html; charset=utf-8"
          import="ru.sibinco.lib.backend.service.ServiceInfo,
                  ru.sibinco.scag.util.LocaleMessages"
 %><jsp:useBean id="bean" class="ru.sibinco.scag.beans.gw.status.Status"
 />status
<%
  bean.process(request, response);
  String label = LocaleMessages.getInstance().getMessage(session,"status.unknown");
    if( !bean.getAppContext().isCluster() ){
  switch(bean.getScagStatus())
  {
    case ServiceInfo.STATUS_RUNNING:
      label = LocaleMessages.getInstance().getMessage(session,"status.running");
      out.print("<img src='content/images/ic_running.gif' width=10 height=10 title='"+label+"'> "+label);
      break;
    case ServiceInfo.STATUS_STOPPING:
      label = LocaleMessages.getInstance().getMessage(session,"status.stopping");
      out.print("<img src='content/images/ic_stopping.gif' width=10 height=10 title='"+label+"'> "+label);
      break;
    case ServiceInfo.STATUS_STOPPED:
      label = LocaleMessages.getInstance().getMessage(session,"status.stopped");
      out.print("<img src='content/images/ic_stopped.gif' width=10 height=10 title='"+label+"'> "+label);
      break;
    case ServiceInfo.STATUS_STARTING:
      label = LocaleMessages.getInstance().getMessage(session,"status.starting");
      out.print("<img src='content/images/ic_starting.gif' width=10 height=10 title='"+label+"'> "+label);
      break;
    default:
      out.print("<img src='content/images/ic_unknown.gif' width=10 height=10 title='"+label+"'> "+label);
      break;
  }
    }else{
        //System.out.println("SCAG STATUS UNK");
        out.print("<img src='content/images/ic_unknown.gif' width=10 height=10 title='"+label+"'> "+label);
    }
%>