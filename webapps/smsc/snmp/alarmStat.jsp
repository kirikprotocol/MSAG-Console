<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.snmp.AlarmStatFormBean,
                 java.text.SimpleDateFormat" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.snmp.AlarmStatRow"%>
<jsp:useBean id="alarmStatBean" scope="session" class="ru.novosoft.smsc.jsp.smsc.snmp.AlarmStatFormBean"/>
<%
    AlarmStatFormBean bean = alarmStatBean;
%>
<jsp:setProperty name="alarmStatBean" property="*"/>
<%
    TITLE = getLocString("smsview.title");
    MENU0_SELECTION = "MENU0_SMSVIEW";

    int beanResult = bean.process(request);
    switch (beanResult) {
        case AlarmStatFormBean.RESULT_DONE:
            response.sendRedirect("alarmStat.jsp");
            return;
        case AlarmStatFormBean.RESULT_FILTER:
        case AlarmStatFormBean.RESULT_OK:
            break;
        case AlarmStatFormBean.RESULT_ERROR:
            break;
        default:
            errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%@ include file="/WEB-INF/inc/calendar.jsp" %>
<div class=content>
<div class=page_subtitle><%=getLocString("common.titles.searchParams")%></div>
<table class=properties_list cell>
<tr class=row0>
    <th><%=getLocString("snmp.viewDate")%></th>
    <td nowrap><input type=text id=date name=date class=calendarField value="<%=bean.getDate()%>"
                      maxlength=20 style="z-index:22;">
        <button class=calendarButton type=button onclick="return showCalendar(date, false, false);">...</button>
    </td>
</tr>
</table>
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbQuery", "common.buttons.queryExcl", "common.buttons.runQuery");
    page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clearQueryParams");
    page_menu_space(out);
    page_menu_end(out);
%>
<div class=content><%
    if (bean.getTotalSizeInt() > 0) {%>
    <input type=hidden name=sort>
    <input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
    <script>
        function setSort(sorting)
        {
            if (sorting == "<%=bean.getSort()%>")
                opForm.sort.value = "-<%=bean.getSort()%>";
            else
                opForm.sort.value = sorting;

            opForm.submit();
            return false;
        }
    </script>
    <br>
    <%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ results ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
    <%
        SimpleDateFormat dateFormatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss", getLoc());
    %>
    <div class=page_subtitle><%=getLocString("common.titles.searchResultsPre") + bean.getTotalSize() + getLocString("common.titles.searchResultsPost")%></div>
    <%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>
    <table class=list cellspacing=0>
        <thead>
            <tr class=row0>
                <th>
                    <a href="javascript:setSort('submit_time')" <%=bean.getSort().endsWith("submit_time") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
                       title="<%=getLocString("snmp.submit_timeHint")%>"><%=getLocString("snmp.submit_time")%></a></th>
                <th>
                    <a href="javascript:setSort('alarm_id')" <%=bean.getSort().endsWith("alarm_id") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
                       title="<%=getLocString("snmp.alarm_idHint")%>"><%=getLocString("snmp.alarm_id")%></a>
                </th>
                <th>
                    <a href="javascript:setSort('alarmcategory')" <%=bean.getSort().endsWith("alarmcategory") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
                       title="<%=getLocString("snmp.alarmcategoryHint")%>"><%=getLocString("snmp.alarmcategory")%></a>
                </th>
                <th>
                    <a href="javascript:setSort('severity')" <%=bean.getSort().endsWith("severity") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
                       title="<%=getLocString("snmp.severityHint")%>"><%=getLocString("snmp.severity")%></a>
                </th>
                <th>
                    <a href="javascript:setSort('text')" <%=bean.getSort().endsWith("text") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
                       title="<%=getLocString("snmp.textHint")%>"><%=getLocString("snmp.text")%></a>
                </th>
            </tr></thead>
        <tbody><%
            int firstIndex = bean.getStartPositionInt() + 1;
            int lastIndex = bean.getStartPositionInt() + bean.getPageSizeInt();
            if (lastIndex >= bean.getTotalSizeInt() || bean.getPageSizeInt() < 0)
                lastIndex = bean.getTotalSizeInt();

            {
                int rowN = 0;
                for (int cnt = firstIndex; cnt <= lastIndex; cnt++, rowN++) {
                    AlarmStatRow row = bean.getRow(cnt - 1);
                    if (row == null) {
                        rowN--;
                        continue;
                    }
        %><tr class=row<%=rowN&1%>0>
            <td nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
                <%= (row.getSubmit_time() != null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getSubmit_time())) : "&nbsp;"%>
            </td>
            <td nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
                <%= (row.getAlarm_id() != null) ? StringEncoderDecoder.encode(row.getAlarm_id()) : "&nbsp;"%>
            </td>
            <td nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
                <%= StringEncoderDecoder.encode(row.getAlarm_category())%><br>
            </td>
            <td nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
                <%= StringEncoderDecoder.encode(Integer.toString(row.getSeverity()))%><br>
            </td>
            <td nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
                <%= StringEncoderDecoder.encode(row.getText())%><br>
            </td>
        </tr>
        <%
                }
            }
        %></tbody>
    </table>
    <%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>
    <% } %>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>