<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.admin.smsstat.ExportSettings,
                 ru.novosoft.smsc.jsp.smsexport.SmsExportBean" %>
<jsp:useBean id="smsExportBean" scope="page" class="ru.novosoft.smsc.jsp.smsexport.SmsExportBean"/>
<% SmsExportBean bean = smsExportBean;%>
<jsp:setProperty name="smsExportBean" property="*"/>
<%
    TITLE = getLocString("smsview.exportTitle");
    MENU0_SELECTION = "MENU0_SMSEXPORT";

    int beanResult = bean.process(request);
    switch (beanResult) {
        case SmsExportBean.RESULT_DONE:
            response.sendRedirect("export.jsp");
            return;
        case SmsExportBean.RESULT_FILTER:
        case SmsExportBean.RESULT_OK:
            break;
        case SmsExportBean.RESULT_ERROR:
            break;
        default:
            errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
    ExportSettings defExpSett = bean.getDefaultExportSettings();
    int rowN = 0;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%@ include file="/WEB-INF/inc/calendar.jsp" %>
<div class=content><%
    if (defExpSett != null && !defExpSett.isEmpty())
    {%>
    <div class=page_subtitle><%=getLocString("smsview.dbSettings")%></div>
    <script>
        function defaultSetting() {
            document.getElementById('source').value = "<%=defExpSett.getSource()%>";
            document.getElementById('driver').value = "<%=defExpSett.getDriver()%>";
            document.getElementById('user').value = "<%=defExpSett.getUser()%>";
            document.getElementById('password').value = "<%=defExpSett.getPassword()%>";
            document.getElementById('tablesPrefix').value = "<%=defExpSett.getTablesPrefix()%>";

            document.getElementById('source').disabled = true;
            document.getElementById('driver').disabled = true;
            document.getElementById('user').disabled = true;
            document.getElementById('password').disabled = true;
            document.getElementById('tablesPrefix').disabled = true;

        }

        function userDefined() {
            document.getElementById('source').disabled = false;
            document.getElementById('driver').disabled = false;
            document.getElementById('user').disabled = false;
            document.getElementById('password').disabled = false;
            document.getElementById('tablesPrefix').disabled = false;
        }
    </script>
    
    <table class=properties_list cell>
        <tr class=row<%= (rowN++) % 2%>>
            <td nowrap><input class=radio type="radio" name="storage" id=operativeStorage
                    value="<%= SmsExportBean.OPERATIVE_STORAGE%>"
            <%= (bean.getStorage() == SmsExportBean.OPERATIVE_STORAGE) ? "checked":""%>
                    onclick="javascript:opForm.submit()">
                <label for=operativeStorage><%=getLocString("common.storages.operative")%></label></td>
            <td nowrap><input class=radio type="radio" name="storage" id=archiveStorage
                    value="<%= SmsExportBean.ARCHIVE_STORAGE%>"
            <%= (bean.getStorage() == SmsExportBean.ARCHIVE_STORAGE) ? "checked":""%>
                onclick="javascript:opForm.submit()">
                <label for=archiveStorage><%=getLocString("common.storages.archive")%></label>
            </td>
        </tr>

        <% if (bean.getStorage() == SmsExportBean.ARCHIVE_STORAGE) { %>
        <tr class=row<%= (rowN++) % 2%>>
            <th><%=getLocString("stat.exportDate")%>:</th>
            <td nowrap><input type=text id="date" name="date" class=calendarField value="<%=bean.getDate()%>"
                              maxlength=20 style="z-index:22;">
                       <button class=calendarButton type=button onclick="return showCalendar(date, false, false);">...</button>
            </td>
        </tr>
        <% } %>

        <tr class=row<%= (rowN++) % 2%>>
            <!-- TODO: add script feature to disable/restore edit fields when switching to DEFAULT destination -->
            <td nowrap><input
                    class=radio type="radio" name="destination" id=destinationDefault
                    value="<%= SmsExportBean.DEFAULT_DESTINATION%>"
            <%= (bean.getDestination() == SmsExportBean.DEFAULT_DESTINATION) ? "checked":""%>
                    onclick="javascript:defaultSetting()">
                <label for=destinationDefault><%=getLocString("common.dests.default")%></label></td>
            <td nowrap><input
                    class=radio type="radio" name="destination" id=destinationUser onclick="javascript:userDefined()"
                    value="<%= SmsExportBean.USER_DESTINATION%>"
            <%= (bean.getDestination() == SmsExportBean.USER_DESTINATION) ? "checked":""%>>
                <label for=destinationUser><%=getLocString("common.dests.user")%></label>
            </td>
        </tr>
        <tr class=row<%= (rowN++) % 2%>>
            <th><%=getLocString("smsview.JDBCsource")%></th>
            <td nowrap><input class=txt type=text id="source" name="source" value="<%=defExpSett.getSource()%>"></td>
        </tr>
        <tr class=row<%= (rowN++) % 2%>>
            <th><%=getLocString("smsview.JDBCdriver")%></th>
            <td nowrap><input class=txt type=text id="driver" name="driver" value="<%=defExpSett.getDriver()%>"></td>
        </tr>
        <tr class=row<%= (rowN++) % 2%>>
            <th><%=getLocString("smsview.user")%></th>
            <td nowrap><input class=txt type=text id="user" name="user" value="<%=defExpSett.getUser()%>"></td>
        </tr>
        <tr class=row<%= (rowN++) % 2%>>
            <th><%=getLocString("smsview.password")%></th>
            <td nowrap><input class=txt type=password id="password" name="password"
                              value="<%=defExpSett.getPassword()%>"></td>
        </tr>
        <tr class=row<%= (rowN++) % 2%>>
            <th><%=getLocString("smsview.tableName")%></th>
            <td nowrap><input class=txt type=text id="tablesPrefix" name="tablesPrefix"
                              value="<%=defExpSett.getTablesPrefix()%>"></td>
        </tr>
    </table>
    <%}
    %>
</div>
<%
    page_menu_begin(out);
    if (errorMessages.size() == 0)
    page_menu_button(session, out, "mbExport", "common.buttons.export", "common.buttons.smsExportHint");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>