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
<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Masks ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=secList>Masks</div></td></tr>
<%
int row = 0;
for (int i=0; i<bean.getMasks().length; i++)
{
%>
<tr class=row<%=(row++)&1%>>
	<th class=label>&nbsp;</th>
	<td><input class=txtW name=masks value="<%=bean.getMasks()[i]%>"></td>
</tr>
<%}%>
<tr class=row<%=(row++)&1%>>
	<th class=label><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to filter"></th>
	<td><input class=txtW name=masks></td>
</tr>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=secView>Options</div></td></tr>
<%row = 0;%>
<tr class=row<%=(row++)&1%>>
	<th class=label>codepage:</th>
	<td><div class=select><select class=txt name=codepage>
			<option value="-1" <%=-1 == bean.getCodepage() ? "selected" : ""%>>all</option>
			<option value="<%=Profile.CODEPAGE_Default%>" <%=Profile.CODEPAGE_Default == bean.getCodepage() ? "selected" : ""%>>Default</option>
			<option value="<%=Profile.CODEPAGE_UCS2   %>" <%=Profile.CODEPAGE_UCS2    == bean.getCodepage() ? "selected" : ""%>>UCS2</option>
		</select></div></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>report&nbsp;mode:</th>
	<td><div class=select><select class=txt name=reportinfo>
			<option value="-1" <%=-1 == bean.getReportinfo() ? "selected" : ""%>>all</option>
			<option value="<%=Profile.REPORT_OPTION_None%>" <%=Profile.REPORT_OPTION_None == bean.getReportinfo() ? "selected" : ""%>>none</option>
			<option value="<%=Profile.REPORT_OPTION_Full%>" <%=Profile.REPORT_OPTION_Full == bean.getReportinfo() ? "selected" : ""%>>full</option>
		</select></div></td>
</tr>
</table>
<div class=secButtons>
<input class=btn type=submit name=mbApply value="Apply" title="Apply filter">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>