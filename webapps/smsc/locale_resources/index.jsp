<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.localeResources.Index"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.localeResources.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Resources";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		STATUS.append("Ok");
		break;
	case Index.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
	case Index.RESULT_FILTER:
		response.sendRedirect("localeResourcesFilter.jsp");
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("localeResourcesAdd.jsp");
		return;
	case Index.RESULT_VIEW:
		response.sendRedirect("localeResourcesView.jsp?locale="+URLEncoder.encode(bean.getViewLocaleResource()));
		return;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_LOCALE_RESOURCES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(out, "mbAdd",  "Add locale resource",  "Add locale resource");
//page_menu_button(out, "mbDelete", "Delete locale resource(s)", "Delete selected locale resource(s)");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
<input type=hidden name=viewLocaleResource>

<script>
function view(locale)
{
	document.all.jbutton.name = "mbView";
	opForm.viewLocaleResource.value = locale;
	opForm.submit();
	return false;
}
function setSort(sorting)
{
	if (sorting == "<%=bean.getSort()%>")
		opForm.sort.value = "-<%=bean.getSort()%>";
	else
		opForm.sort.value = sorting;
	opForm.submit();
	return false;
}
function navigate(direction)
{
	document.all.jbutton.name = direction;
	document.all.jbutton.value = direction;
	opForm.submit();
	return false;
}
</script>
<table class=list cellspacing=0>
<%--col width="1%"--%>
<thead>
<tr>
	<%--th>&nbsp;</th--%>
 	<th><a href="#" <%=bean.getSort().endsWith("locale") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by mask" onclick='return setSort("locale")'>locale name</a></th>
</tr>
</thead>
<tbody>
<%{
int row = 0;
for(Iterator i = bean.getLocales().iterator(); i.hasNext();)
{
String locale = (String) i.next();
String encLocale = StringEncoderDecoder.encode(locale);
%>
<tr class=row<%=(row++)&1%>>
	<%--td class=check><input class=check type=checkbox name=checkedLocales value="<%=encLocale%>" <%=bean.isLocaleChecked(locale) ? "checked" : ""%>></td--%>
	<td class=name><a href="#" title="View contens of resource file" onClick='return view("<%=encLocale%>")'><%=encLocale%></a></td>
</tr>
<%}}%>
</tbody>
</table>
<%--@ include file="/WEB-INF/inc/navbar.jsp"--%>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbAdd",  "Add locale resource",  "Add locale resource");
//page_menu_button(out, "mbDelete", "Delete locale resource(s)", "Delete selected locale resource(s)");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
