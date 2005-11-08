<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.localeResources.Index,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 java.net.URLEncoder"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.localeResources.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("locale.title");
switch(bean.process(request))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		break;
	case Index.RESULT_ERROR:
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
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_LOCALE_RESOURCES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",  "locale.add",  "locale.add");
//page_menu_button(out, "mbDelete", "locale.delete", "locale.deleteHint");
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
 	<th><a href="#" <%=bean.getSort().endsWith("locale") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.localeHint")%>" onclick='return setSort("locale")'><%=getLocString("common.sortmodes.locale")%></a></th>
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
	<td class=name><a href="#" title="<%=getLocString("locale.viewSubTitle")%>" onClick='return view("<%=encLocale%>")'><%=encLocale%></a></td>
</tr>
<%}}%>
</tbody>
</table>
<%--@ include file="/WEB-INF/inc/navbar.jsp"--%>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",  "locale.add",  "locale.add");
//page_menu_button(out, "mbDelete", "locale.delete", "locale.deleteHint");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
