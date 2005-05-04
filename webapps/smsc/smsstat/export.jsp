<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, java.text.SimpleDateFormat,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors"%>
<%@ page import="ru.novosoft.smsc.admin.smsstat.*"%>
<%@ page import="ru.novosoft.smsc.jsp.smsstat.*"%>
<jsp:useBean id="statExportBean" scope="session" class="ru.novosoft.smsc.jsp.smsstat.StatExportBean" />
<%
	StatExportBean bean = statExportBean;
    bean.setDate(null);
%>
<jsp:setProperty name="statExportBean" property="*"/>
<%
    TITLE="SMS Statistics export";
    MENU0_SELECTION = "MENU0_SMSSTAT";

    int beanResult = StatExportBean.RESULT_OK;
    switch(beanResult = bean.process(request))
    {
        case StatExportBean.RESULT_DONE:
            response.sendRedirect("export.jsp");
            return;
        case SmsStatFormBean.RESULT_OK:
            break;
        case SmsStatFormBean.RESULT_ERROR:
            break;
        default:
            errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>

<div class=page_subtitle>Export statistics</div>
<table class=properties_list cellspacing=0>
<%int rowN = 0;%>
<tr class=row<%= rowN%>>
	<th>Date:</th>
	<td nowrap><input type=text id="date" name="date" class=calendarField value="<%=bean.getDate()%>" maxlength=10 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(date, false, true);">...</button></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
</tr>
</table>

<div class=page_subtitle>Export destination</div>
<table class=properties_list cellspacing=0>
<tr class=row<%= (rowN++)%2%>>
	<th>JDBC source</th>
	<td nowrap><input class=txt type=text name="source" value="<%=bean.getSource()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>JDBC driver</th>
	<td nowrap><input class=txt type=text name="driver" value="<%=bean.getDriver()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>User</th>
	<td nowrap><input class=txt type=text name="user" value="<%=bean.getUser()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>Password</th>
	<td nowrap><input class=txt type=password name="password" value="<%=bean.getPassword()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>Stat tables prefix</th>
	<td nowrap><input class=txt type=text name="tablesPrefix" value="<%=bean.getTablesPrefix()%>"></td>
</tr>
</table>
</div> <!-- content -->
<%
page_menu_begin(out);
page_menu_button(out, "mbExport",  "Export !",  "Process export statistics to selected destination");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
