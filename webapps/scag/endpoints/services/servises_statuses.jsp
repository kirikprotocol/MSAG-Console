<%@ page  pageEncoding="UTF-8"
          contentType="text/html; charset=utf-8"
          import="ru.sibinco.scag.backend.endpoints.svc.SvcStatus,ru.sibinco.scag.util.LocaleMessages" %><%@
        page import="java.util.Iterator" %><%@
        page import="java.util.List"
        %><%@
        page import="java.util.Map"
        %><jsp:useBean id="bean" class="ru.sibinco.scag.beans.endpoints.EndpointStatus"/><%
    bean.process(request, response);
    String label = "";
    List c = bean.getSvcIds();
    String result = "";

    for (Iterator i = c.iterator(); i.hasNext();) {
        result += '"' + (String) i.next() + '"';

        if (i.hasNext()) {
            result += ", ";
        }
    }
    result += ", ";
    for (Iterator i = c.iterator(); i.hasNext();) {
        result += '"' + (String) i.next() + "_s" + '"';

        if (i.hasNext()) {
            result += ", ";
        }
    }
    result += "\n";
    Map status = bean.getSvcStatuses();
    for (Iterator i = c.iterator(); i.hasNext();) {
        String sId = (String) i.next();
        SvcStatus svcStatus = (SvcStatus) status.get(sId);
	String connHost = (svcStatus != null && svcStatus.getConnHost() != null) ?
			   svcStatus.getConnHost():"&nbsp;";
        result += "<span " + ">" + connHost + "</span>";
        if (i.hasNext()) {
            result += ", ";
        }
    }
    result += ", ";
    Map status2 = bean.getSvcStatuses();
    for (Iterator i = c.iterator(); i.hasNext();) {
        String sId = (String) i.next();
        SvcStatus svcStatus = (SvcStatus) status2.get(sId);
        result += "<span ";
        if( svcStatus != null && status != null ) {
            if (svcStatus.getConnStatus().equals("yes")) {
                label = LocaleMessages.getInstance().getMessage(session,"centers.index.table.names.connected");
                result += "><img src='content/images/ic_internal.gif' width='10' height='10' title='"+label+"'></span>";
            } else if (svcStatus.getConnStatus().equals("no")) {
                label = LocaleMessages.getInstance().getMessage(session,"centers.index.table.names.disconnected");
                result += "><img src='content/images/ic_disconnct.gif' width='10' height='10' title='"+label+"'></span>";
            } else if (svcStatus.getConnStatus().equals("unknown")) {
                label = LocaleMessages.getInstance().getMessage(session,"centers.index.table.names.unknown");
                result += "><img src='content/images/ic_unknown3.bmp' width='10' height='10' title='"+label+"'></span>";
            }
        } else {
            //System.out.println("SERRVICE STATUS UNK");
            result += " class=C000>unknown</span>";
        }
        if (i.hasNext()) {
            result += ", ";
        }
    }
    out.print(result.trim());%>
