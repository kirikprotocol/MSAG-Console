<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.users.UsersFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.users.UsersFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Filter users";
switch(bean.process(appContext, errorMessages))
{
	case UsersFilter.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case UsersFilter.RESULT_OK:
		STATUS.append("Ok");
		break;
	case UsersFilter.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_USERS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<input type="Hidden" name=initialized value="true">
<%int rowN = 0;%>
<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<%--<tr><td colspan=2 class=secInner><div class=secList>Default SMEs</div></td></tr>--%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>name</th>
	<td><input title="Use the regexp format (.* as wildcart, for example)" class=txtW name=loginFilter value="<%=bean.getLoginFilter()%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>first name</th>
	<td><input title="Use the regexp format (.* as wildcart, for example)" class=txtW name=firstNameFilter value="<%=bean.getFirstNameFilter()%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>last name</th>
	<td><input title="Use the regexp format (.* as wildcart, for example)" class=txtW name=lastNameFilter value="<%=bean.getLastNameFilter()%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>department</th>
	<td><input title="Use the regexp format (.* as wildcart, for example)" class=txtW name=departmentFilter value="<%=bean.getDepartmentFilter()%>"></td>
</tr>
</table>
<div class=secButtons>
<input class=btn type=submit name=mbApply value="Apply" title="Apply filter">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>