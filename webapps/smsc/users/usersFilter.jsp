<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.users.UsersFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.users.UsersFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Filter users";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
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
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter");
page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<input type="Hidden" name=initialized value="true">
<%int rowN = 0;%>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="85%">
<%--<tr><td colspan=2 class=secInner><div class=secList>Default SMEs</div></td></tr>--%>
<tr class=row<%=(rowN++)&1%>>
	<th>name:</th>
	<td><input title="Use the regexp format (.* as wildcart, for example)" class=txt name=loginFilter value="<%=bean.getLoginFilter()%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>first name:</th>
	<td><input title="Use the regexp format (.* as wildcart, for example)" class=txt name=firstNameFilter value="<%=bean.getFirstNameFilter()%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>last name:</th>
	<td><input title="Use the regexp format (.* as wildcart, for example)" class=txt name=lastNameFilter value="<%=bean.getLastNameFilter()%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>department:</th>
	<td><input title="Use the regexp format (.* as wildcart, for example)" class=txt name=departmentFilter value="<%=bean.getDepartmentFilter()%>"></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter");
page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>