<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesFilter,
                 ru.novosoft.smsc.admin.profiler.Profile"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case ProfilesFilter.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ProfilesFilter.RESULT_OK:
		STATUS.append("Ok");
		break;
	case ProfilesFilter.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROFILES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<h1>Profiles filter</h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
<table class=frm0 cellspacing=0 width="100%">
<col width="10%" align=right>
<col width="85%">
<col width="10%">
<%
for (int i=0; i<bean.getMasks().length; i++)
{
%>
<tr class=row0>
	<th>mask:</th>
	<td><input class=txtW name=masks value="<%=bean.getMasks()[i]%>"></td>
	<td>&nbsp;</td>
</tr>
<%}%>
<tr class=row0>
	<th>mask:</th>
	<td><input class=txtW name=masks></td>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to filter"></td>
</tr>
<tr class=row0>
	<th>codepage:</th>
	<td><select class=txt name=codepage>
			<option value="-1" <%=-1 == bean.getCodepage() ? "selected" : ""%>>all</option>
			<option value="<%=Profile.CODEPAGE_Default%>" <%=Profile.CODEPAGE_Default == bean.getCodepage() ? "selected" : ""%>>Default</option>
			<option value="<%=Profile.CODEPAGE_UCS2   %>" <%=Profile.CODEPAGE_UCS2    == bean.getCodepage() ? "selected" : ""%>>UCS2</option>
		</select></td>
	<td>&nbsp;</td>
</tr>
<tr class=rowLast>
	<th>report&nbsp;mode:</th>
	<td><select class=txt name=reportinfo>
			<option value="-1" <%=-1 == bean.getReportinfo() ? "selected" : ""%>>all</option>
			<option value="<%=Profile.REPORT_OPTION_None%>" <%=Profile.REPORT_OPTION_None == bean.getReportinfo() ? "selected" : ""%>>none</option>
			<option value="<%=Profile.REPORT_OPTION_Full%>" <%=Profile.REPORT_OPTION_Full == bean.getReportinfo() ? "selected" : ""%>>full</option>
		</select></td>
	<td>&nbsp;</td>
</tr>
</table>
<div class=but0>
<input class=btn type=submit name=mbApply value="Apply" title="Apply filter">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>