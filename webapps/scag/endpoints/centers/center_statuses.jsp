<%@ page  pageEncoding="UTF-8"
          contentType="text/html; charset=utf-8"
          import="ru.sibinco.scag.backend.endpoints.centers.CenterStatus,ru.sibinco.scag.util.LocaleMessages" %><%@
        page import="java.util.Iterator" %><%@
        page import="java.util.List"
        %><%@
        page import="java.util.Map"
        %><jsp:useBean id="bean" class="ru.sibinco.scag.beans.endpoints.EndpointStatus"/><%
    bean.process(request, response);
    String label = "";
    List c = bean.getCenterIds();
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
    Map status = bean.getCenterStatuses();
    for (Iterator i = c.iterator(); i.hasNext();) {
        String sId = (String) i.next();
        CenterStatus centerStatus = (CenterStatus) status.get(sId);
	String hostPort = (centerStatus != null && centerStatus.getConnHostPort() != null) ?
			   centerStatus.getConnHostPort():"&nbsp;";
        result += "<span " + ">" + hostPort + "</span>";
        if (i.hasNext()) {
            result += ", ";
        }
    }
    result += ", ";
    Map status2 = bean.getCenterStatuses();
    for (Iterator i = c.iterator(); i.hasNext();) {
        String sId = (String) i.next();
        CenterStatus centerStatus = (CenterStatus) status2.get(sId);
        result += "<span ";
        if( centerStatus != null && status != null) {
            if (centerStatus.getConnStatus().equals("yes")) {
                label = LocaleMessages.getInstance().getMessage(session,"centers.index.table.names.connected");
                result += "><img src='content/images/ic_internal.gif' width='10' height='10' title='"+label+"'></span>";
            } else if (centerStatus.getConnStatus().equals("no")) {
                label = LocaleMessages.getInstance().getMessage(session,"centers.index.table.names.disconnected");
                result += "><img src='content/images/ic_disconnct.gif' width='10' height='10' title='"+label+"'></span>";
            } else if (centerStatus.getConnStatus().equals("unknown")) {
                label = LocaleMessages.getInstance().getMessage(session,"centers.index.table.names.unknown");
                result += "><img src='content/images/ic_unknown3.bmp' width='10' height='10' title='"+label+"'></span>";
            }
        } else {
            //System.out.println("CENTER STATUS UNK");
            result += " class=C000>unknown</span>";
        }
        if (i.hasNext()) {
            result += ", ";
        }
    }
    out.print(result.trim());%>
