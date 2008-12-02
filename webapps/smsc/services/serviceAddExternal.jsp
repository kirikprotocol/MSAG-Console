<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.services.ServiceAddInternal" %>
<%@ page import="java.util.*" %>
<%@ page import="ru.novosoft.smsc.admin.daemon.Daemon" %>
<%@ page import="java.io.*" %>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.ServiceAddInternal"/>
<jsp:setProperty name="bean" property="*"/>
<%
    if (Constants.instType != ResourceGroupConstants.RESOURCEGROUP_TYPE_SINGLE)
        TITLE = getLocString("services.add");
    else
        TITLE = getLocString("services.addTitle2");
    switch (bean.process(request)) {
        case ServiceAddInternal.RESULT_DONE:
            if (Constants.instType != ResourceGroupConstants.RESOURCEGROUP_TYPE_SINGLE)
                response.sendRedirect("resourceGroups.jsp");
            else {
              SMSCAppContext ctx = (SMSCAppContext) request.getAttribute("appContext");
              SME sme = ctx.getSmeManager().get(bean.getServiceId());
              java.util.List hnames = ctx.getHostsManager().getDaemonManager().getHostNames();
              String hostName = (String) hnames.get(0);
              Daemon daemon = ctx.getHostsManager().getDaemonManager().get((String) hnames.get(0));
              ServiceInfo serviceInfo =new ServiceInfo(bean.getServiceId(), hostName,
                                ctx.getHostsManager().getDaemonServicesFolder(hostName) + java.io.File.separatorChar + bean.getServiceId(),
                                "", false, sme,
                                ServiceInfo.STATUS_OFFLINE);
              daemon.addService(serviceInfo);
              response.sendRedirect("index.jsp");
            }
            return;
        case ServiceAddInternal.RESULT_OK:

            break;
        case ServiceAddInternal.RESULT_ERROR:

            break;
    }
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
    MENU0_SELECTION = "MENU0_SERVICES";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp" %>
<%int rowN = 0;
    page_menu_begin(out);
    page_menu_button(session, out, "mbNext", "common.buttons.finish", "services.add");
    page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "return clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<div class=content>
    <div class=page_subtitle><%=getLocString("services.addSubTitle")%>:</div>
    <table class=properties_list cellspacing=0 cellpadding=0>
        <col width="15%" align=right>
        <col width="85%">
        <tr class=row<%=(rowN++) & 1%>>
            <th><%=getLocString("services.systemId")%>:</th>
            <td><input class=txt type="text" name="serviceId"
                       value="<%=StringEncoderDecoder.encode(bean.getServiceId())%>"></td>
        </tr>
        <%@ include file="serviceSmeBody.jsp" %>
    </table>
    <%if (bean.getHostName() != null) {
    %><input class=txt type=hidden name=hostName value="<%=bean.getHostName()%>"><%
    }%>
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbNext", "common.buttons.finish", "services.add");
    page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "return clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>
