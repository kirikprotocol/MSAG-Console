<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.services.Index,
                 ru.novosoft.smsc.jsp.smsc.services.ResGroups,
                 java.util.Arrays,
                 java.util.List" %>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.ResGroups"/>
<jsp:setProperty name="bean" property="*"/>
<%
    TITLE = getLocString("services.title");
    isServiceStatusNeeded = true;
    switch (bean.process(request)) {
    case Index.RESULT_DONE:
        response.sendRedirect("resourceGroups.jsp");
        return;
    case Index.RESULT_OK:

        break;
    case Index.RESULT_ERROR:

        break;
    case Index.RESULT_VIEW:
        response.sendRedirect(CPATH + "/esme_" + URLEncoder.encode(bean.getServiceId()) + "/index.jsp");
        return;
    case ResGroups.RESULT_VIEW_NODES:
        response.sendRedirect(CPATH + "/hosts/nodesView.jsp?servName=" + bean.getServiceId());
        return;
    case Index.RESULT_ADD:
        response.sendRedirect("serviceAddInternal.jsp" + (bean.getHostId() != null ? ("?hostName=" + bean.getHostId()) : ""));
        return;
    case Index.RESULT_EDIT:
        response.sendRedirect(CPATH + "/services/serviceEditSme.jsp?serviceId=" + URLEncoder.encode(bean.getServiceId()));
        return;
    default:

        errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
    MENU0_SELECTION = "MENU0_SERVICES";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbAddService", "common.buttons.add", "services.add");
    page_menu_confirm_button(session, out, "mbDelete", "common.buttons.delete", "services.deleteHint", getLocString("services.deleteConfirm"));
    page_menu_space(out);
    page_menu_confirm_button(session, out, "mbDisconnectServices", "common.buttons.disconnect", "services.disconnectHint", getLocString("services.disconnectConfirm"), null, bean.isSmscAlive());
    page_menu_button(session, out, "mbStartService", "common.buttons.online", "common.hints.online");
    page_menu_button(session, out, "mbStopService", "common.buttons.offline", "common.hints.offline");
    page_menu_button(session, out, "mbSwitchOver", "common.buttons.switchOver", "common.hints.switchOver");
    page_menu_end(out);
%><div class=content>
    <input type=hidden name=serviceId>
    <input type=hidden name=hostId>
    <input type=hidden name=serviceType>
    <script>
        function viewNodes(servId)
        {
            document.getElementById('jbutton').value = "view host";
            document.getElementById('jbutton').name = "mbViewNodes";
            opForm.serviceId.value = servId;
            opForm.submit();
            return false;
        }
        function viewService(serviceId)
        {
            document.getElementById('jbutton').value = "view";
            document.getElementById('jbutton').name = "mbView";
            opForm.serviceId.value = serviceId;
            opForm.submit();
            return false;
        }
        function editService(serviceId)
        {
            document.getElementById('jbutton').value = "edit";
            document.getElementById('jbutton').name = "mbEdit";
            opForm.serviceId.value = serviceId;
            opForm.submit();
            return false;
        }
    </script>

    <table class=list cellspacing=1 width="100%">
        <col width="1%" align=center>
        <%if (request.isUserInRole("services")) {%><col width="1%"><%}%>
        <col width="60%" align=left>
        <col width="1%" align=center>
        <col width="20%" align=left>
        <col width="10%" align=center>
        <thead>
            <tr>
                <th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
                <%if (request.isUserInRole("services")) {%><th>&nbsp;</th><%}%>
                <th><%=getLocString("common.sortmodes.service")%></th>
                <th colspan="2"><%=getLocString("common.sortmodes.status")%></th>
                <th><%=getLocString("common.sortmodes.runstatus")%></th>
            </tr>
        </thead>
        <tbody>
            <%{
                int row = 0;
                for (Iterator i = bean.getSmeIds().iterator(); i.hasNext(); row++) {
                    String serviceId = (String) i.next();
                    String encodedServiceId = StringEncoderDecoder.encode(serviceId);

                    List serviceIds = Arrays.asList(bean.getServiceIds());
            %>
            <tr class=row<%=row & 1%>>
                <td class=check width="1%"><input class=check type=checkbox name=serviceIds
                                                  value="<%=encodedServiceId%>" <%=serviceIds.contains(serviceId) ? "checked" : ""%>>
                </td><%
                if (request.isUserInRole("services")) {
            %><td class=name width="1%" nowrap><a href="javascript:editService('<%=encodedServiceId%>')"
                                                  title="<%=getLocString("services.editSubTitle")%>"><%=getLocString("common.links.edit")%></a>
            </td><%
                }
            %><td class=name><%
                if (bean.isServiceAdministrable(serviceId) && request.isUserInRole("services")) {
            %><a href="javascript:viewService('<%=encodedServiceId%>')"
                 title="<%=getLocString("host.viewServInfo")%>"><%=encodedServiceId%></a><%
            }
            else {
            %><%=encodedServiceId%><%
                }
            %></td>
                <td class=name width="1%">
                    <%= bean.isServiceDisabled(serviceId)
                            ?
                            "<img src=\"/images/ic_disable.gif\" width=10 height=10 title='" + getLocString("common.hints.disabled") + "'>"
                            :
                            "<img src=\"/images/ic_enable.gif\" width=10 height=10 title='" + getLocString("common.hints.enabled") + "'>"%></td>
                <td class=name width="1%" nowrap><%=smeStatus(bean.getAppContext(), serviceId)%></td>
                <td class=name width="1%" nowrap><%
                    if (bean.isService(serviceId)) {
                %><%=serviceStatus(bean.getAppContext(), serviceId)%><%
                }
                else {
                %>&nbsp;<%
                    }
                %></td>
            </tr>
            <%}
            }%>
        </tbody>
    </table>
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbAddService", "common.buttons.add", "services.add");
    page_menu_confirm_button(session, out, "mbDelete", "common.buttons.delete", "services.deleteHint", getLocString("services.deleteConfirm"));
    page_menu_space(out);
    page_menu_confirm_button(session, out, "mbDisconnectServices", "common.buttons.disconnect", "services.disconnectHint", getLocString("services.disconnectConfirm"), null, bean.isSmscAlive());
    page_menu_button(session, out, "mbStartService", "common.buttons.online", "common.hints.online");
    page_menu_button(session, out, "mbStopService", "common.buttons.offline", "common.hints.offline");
    page_menu_button(session, out, "mbSwitchOver", "common.buttons.switchOver", "common.hints.switchOver");
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>
