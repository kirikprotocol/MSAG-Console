<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.util.config.Config,
					  java.util.Set,
					  java.util.Iterator,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  ru.novosoft.smsc.jsp.util.tables.DataItem,
					  ru.novosoft.smsc.dbsme.Providers,
					  java.net.URLEncoder"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.dbsme.Providers" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = Constants.WSME_SME_ID;
	TITLE="DB SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.RESULT_OK;
	switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
	{
		case Providers.RESULT_OK:
			STATUS.append("Ok");
			break;
		case Providers.RESULT_DONE:
			response.sendRedirect("providers.jsp");
			return;
		case Providers.RESULT_OVERVIEW:
			response.sendRedirect("index.jsp");
			return;
		case Providers.RESULT_PARAMS:
		case Providers.RESULT_DRIVERS:
		case Providers.RESULT_PROVIDERS:
		case Providers.RESULT_PROVIDER:
			response.sendRedirect(bean.getMenuSelection() + ".jsp");
			return;
		case Providers.RESULT_ADD:
			response.sendRedirect("provider.jsp?creating=true");
			return;
		case Providers.RESULT_EDIT:
			response.sendRedirect("provider.jsp?providerName=" + URLEncoder.encode(bean.getProviderName()));
			return;
		case Providers.RESULT_ERROR:
			STATUS.append("<span class=CF00>Error</span>");
			break;
		default:
			STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/menu.jsp"%>
<div class=content>
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
function edit(pName)
{
	opForm.providerName.value = pName;
	opForm.jbutton.value = "Edit";
	opForm.jbutton.name =  "mbEdit";
	opForm.submit();
	return false;
}
</script>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
<input type=hidden name=providerName>
<table class=list cellspacing=1>
<col width="1%">
<thead>
<tr>
	<th>&nbsp;</th>
	<th><a href="#" <%=bean.getSort().endsWith("name") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by name" onclick='return setSort("name")'>name</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("address") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by address" onclick='return setSort("address")'>address</a></th>
</tr>
</thead>
<tbody>
<%
	int row = 0;
	for (Iterator i = bean.getProviders().iterator(); i.hasNext();) {
		DataItem item = (DataItem) i.next();
      String dpName = (String) item.getValue("name");
		String dpAddress = (String) item.getValue("address");
		String encDpName = StringEncoderDecoder.encode(dpName);
		String encDpAddress = StringEncoderDecoder.encode(dpAddress);
%>
<tr class=row<%=(row++)&1%>>
	<td><input class=check type=checkbox name=checked value="<%=encDpName%>" <%=bean.isDpChecked(dpName) ? "checked" : ""%>></td>
	<td><a href="#" title="Edit provider" onClick='return edit("<%=encDpName%>")'><%=encDpName%></a></td>
	<td><%=encDpAddress%></td>
</tr>
<%
	}
%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbAdd",    "Add",    "Create new data provider");
page_menu_button(out, "mbDelete", "Delete", "Delete all checked data providers", "return confirm('Are you sure to delete all checked data providers?')");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>