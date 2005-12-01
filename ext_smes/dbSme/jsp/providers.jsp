<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.util.config.Config,
					  java.util.Set,
					  java.util.Iterator,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  ru.novosoft.smsc.jsp.util.tables.DataItem,
					  ru.novosoft.smsc.dbsme.Providers,
					  java.net.URLEncoder,
                 ru.novosoft.smsc.jsp.PageBean"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.dbsme.Providers" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("dbsme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.process(request);
	switch(beanResult)
	{
    case PageBean.RESULT_OK:
    case PageBean.RESULT_ERROR:
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
		default:
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
<col width="49%">
<col width="49%">
<col width="1%">
<thead>
<tr>
	<th>&nbsp;</th>
	<th><a href="#" <%=bean.getSort().endsWith("name"   ) ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("dbsme.sort.name")%>"    onclick='return setSort("name")'   ><%= getLocString("dbsme.label.name")%></a></th>
	<th><a href="#" <%=bean.getSort().endsWith("address") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("dbsme.sort.address")%>" onclick='return setSort("address")'><%= getLocString("dbsme.label.address")%></a></th>
    <th><a href="#" <%=bean.getSort().endsWith("enabled") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("dbsme.sort.e_flag")%>"  onclick='return setSort("enabled")'><%= getLocString("dbsme.label.enabled")%></a></th>
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
        boolean dpEnabled = ((Boolean)item.getValue("enabled")).booleanValue();
%>
<tr class=row<%=(row++)&1%>>
	<td><input class=check type=checkbox name=checked value="<%=encDpName%>" <%=bean.isDpChecked(dpName) ? "checked" : ""%>></td>
	<td><a href="#" title="<%= getLocString("dbsme.label.edit_provider")%>" onClick='return edit("<%=encDpName%>")'><%=encDpName%></a></td>
	<td><%=encDpAddress%></td>
  <td align=center><%=dpEnabled ? "<img src=\"/images/ic_checked.gif\">": "&nbsp;"%></td>
</tr>
<%
	}
%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",     "dbsme.button.add_provider",   "dbsme.hint.add_provider");
page_menu_button(session, out, "mbDelete",  "dbsme.hint.delete_providers", "dbsme.hint.delete_providers", "return confirm('"+getLocString("dbsme.confirm.delete_providers")+"')");
page_menu_space(out);
page_menu_button(session, out, "mbEnable",  "dbsme.button.enable",  "dbsme.hint.enable_providers",  !bean.isConfigChanged());
page_menu_button(session, out, "mbDisable", "dbsme.button.disable", "dbsme.hint.disable_providers", !bean.isConfigChanged());
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>