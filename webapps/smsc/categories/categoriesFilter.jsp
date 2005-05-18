<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.providers.ProvidersFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.categories.CategoriesFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Filter Categories";
switch(bean.process(request))
{
	case ProvidersFilter.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ProvidersFilter.RESULT_OK:

		break;
	case ProvidersFilter.RESULT_ERROR:

		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_CATEGORIES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(session, out, "mbClear", "Clear", "Clear filter");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel filter editing");
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
	<td><input title="Use the regexp format (.* as wildcart, for example)" class=txt name=loginFilter value="<%=bean.getNameFilter()%>"></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(session, out, "mbClear", "Clear", "Clear filter");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel filter editing");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>