<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesEdit,
                 ru.novosoft.smsc.admin.profiler.Profile"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case ProfilesEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ProfilesEdit.RESULT_OK:
		STATUS.append("Ok");
		break;
	case ProfilesEdit.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROFILES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<h1>Edit profile <%=bean.getMask()%></h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>

<input type=hidden name="mask" value="<%=bean.getMask()%>">
<table class=frm0 cellspacing=0 width="100%">
<col width="15%" align=right>
<col width="85%">
<tr class=row0>
	<th>codepage:</th>
	<td><select class=txt name=codepage>
			<option value="<%=Profile.CODEPAGE_Default%>" <%=bean.getCodepage() == Profile.CODEPAGE_Default ? "SELECTED" : ""%>>Default</option>
			<option value="<%=Profile.CODEPAGE_UCS2   %>" <%=bean.getCodepage() == Profile.CODEPAGE_UCS2    ? "SELECTED" : ""%>>UCS2</option>
		</select></td>
</tr>
<tr class=rowLast>
	<th>report&nbsp;mode:</th>
	<td><select class=txt name="report">
			<option value="<%=Profile.REPORT_OPTION_None%>" <%=bean.getReport() == Profile.REPORT_OPTION_None ? "SELECTED" : ""%>>none</option>
			<option value="<%=Profile.REPORT_OPTION_Full%>" <%=bean.getReport() == Profile.REPORT_OPTION_Full ? "SELECTED" : ""%>>full</option>
		</select></td>
</tr>
</table>
<div class=but0>
<input class=btn type=submit name=mbSave value="Save" title="Save changes">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>