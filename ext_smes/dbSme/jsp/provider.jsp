<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.util.config.Config,
					  java.util.Set,
					  java.util.Iterator,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  ru.novosoft.smsc.dbsme.Provider,
					  ru.novosoft.smsc.jsp.util.tables.DataItem,
					  java.net.URLEncoder,
					  ru.novosoft.smsc.jsp.PageBean"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.dbsme.Provider" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = Constants.WSME_SME_ID;
	TITLE="DB SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.RESULT_OK;
	switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
	{
		case Provider.RESULT_OK:
			STATUS.append("Ok");
			break;
		case Provider.RESULT_EDIT:
			response.sendRedirect("job.jsp?providerName=" + URLEncoder.encode(bean.getProviderName())
										 + "&jobId=" + URLEncoder.encode(bean.getEdit()));
			return;
		case Provider.RESULT_ADD:
			response.sendRedirect("job.jsp?providerName=" + URLEncoder.encode(bean.getProviderName()) + "&creating=true");
			return;
		case Provider.RESULT_DONE:
			response.sendRedirect("providers.jsp");
			return;
		case Provider.RESULT_REFRESH:
			response.sendRedirect("provider.jsp?providerName=" + URLEncoder.encode(bean.getProviderName()));
			return;
		case Provider.RESULT_OVERVIEW:
			response.sendRedirect("index.jsp");
			return;
		case Provider.RESULT_PARAMS:
		case Provider.RESULT_DRIVERS:
		case Provider.RESULT_PROVIDERS:
		case Provider.RESULT_PROVIDER:
			response.sendRedirect(bean.getMenuSelection() + ".jsp");
			return;
		case Provider.RESULT_ERROR:
			STATUS.append("<span class=CF00>Error</span>");
			break;
		default:
			STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/menu.jsp"%>
<%@include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<input type=hidden name=oldProviderName value="<%=StringEncoderDecoder.encode(bean.getOldProviderName())%>">
<input type=hidden name=edit value="<%=StringEncoderDecoder.encode(bean.getEdit() == null ? "" : bean.getEdit())%>">
<input type=hidden name=creating value="<%=bean.isCreating()%>">
<input type=hidden name=initialized value=true>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
<%
	if (beanResult == PageBean.RESULT_OK) {
%>
<div class=content>
<div class=page_subtitle><%=StringEncoderDecoder.encode(bean.isCreating() ? "New provider" : bean.getProviderName())%></div>
<%
startSection(out, "provider_properties", "Properties", true);
	startParams(out);
		param(out, "name", "providerName", bean.getProviderName());
		param(out, "address", "address", bean.getAddress(), null, null, false, "mask");
		paramSelect(out, "type", "type", bean.getTypes(), bean.getType());
		param(out, "connections", "connections", bean.getConnections(), null, null, false, "positive");
		param(out, "dbInstance", "dbInstance", bean.getDbInstance());
		param(out, "dbUserName", "dbUserName", bean.getDbUserName());
		param(out, "dbUserPassword", "dbUserPassword", bean.getDbUserPassword());
		param(out, "watchdog", "watchdog", bean.isWatchdog());
	finishParams(out);
finishSection(out);
startSection(out, "Messages", "Messages", false);
	startParams(out);
		param(out, "JOB_NOT_FOUND", "job_not_found", bean.getJob_not_found());
		param(out, "DS_FAILURE", "ds_failure", bean.getDs_failure());
		param(out, "DS_CONNECTION_LOST", "ds_connection_lost", bean.getDs_connection_lost());
		param(out, "DS_STATEMENT_FAIL", "ds_statement_fail", bean.getDs_statement_fail());
		param(out, "QUERY_NULL", "query_null", bean.getQuery_null());
		param(out, "INPUT_PARSE", "input_parse", bean.getInput_parse());
		param(out, "OUTPUT_FORMAT", "output_format", bean.getOutput_format());
		param(out, "INVALID_CONFIG", "invalid_config", bean.getInvalid_config());
	finishParams(out);
finishSection(out);
startSection(out, "Jobs", "Jobs", true);
%>
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
</script>
<table class=list cellspacing=0>
<col width=1%>
<tr>
	<th>&nbsp;</th>
	<th><a href="#" <%=bean.getSort().endsWith("name")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by name"    onclick='return setSort("name")'   >name</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("type")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by type"    onclick='return setSort("type")'   >type</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("address") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by address" onclick='return setSort("address")'>address</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("alias")   ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by alias"   onclick='return setSort("alias")'  >alias</a></th>
</tr>
<%
	int row = 0;
	for (Iterator i = bean.getJobs().iterator(); i.hasNext();) {
		DataItem item = (DataItem) i.next();
    String jobId = (String) item.getValue("id");
    String jobName    = (String) item.getValue("name");
		String jobType    = (String) item.getValue("type");
		String jobAddress = (String) item.getValue("address");
		String jobAlias   = (String) item.getValue("alias");

    String encId = StringEncoderDecoder.encode(jobId);
		String encName = StringEncoderDecoder.encode(jobName);
		String encType = StringEncoderDecoder.encode(jobType);
		String encAddress = StringEncoderDecoder.encode(jobAddress);
		String encAlias   = StringEncoderDecoder.encode(jobAlias);
%>
<tr class=row<%=(row++)&1%>>
	<td><input class=check type=checkbox name=checked value="<%=encId%>" <%=bean.isJobChecked(jobId) ? "checked" : ""%>></td>
	<td style="cursor:hand;" onClick='return editSomething("<%=encId%>")' title="Edit job <%=encName%>"><a href="#"><%=encName%></a></td>
	<td><%=encType%>&nbsp;</td>
	<td><%=encAddress%>&nbsp;</td>
	<td><%=encAlias%>&nbsp;</td>
</tr>
<%
	}
%>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
<%finishSection(out);%>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbAdd",    "Add job",    "Create new job");
page_menu_button(out, "mbDelete", "Delete jobs", "Delete all checked jobs", "return confirm('Are you sure to delete all checked jobs?') && confirm('Save provider changes?')");
page_menu_button(out, "mbDone",  "Done",  "");
page_menu_button(out, "mbCancel", "Cancel", "Cancel changes", "return noValidationSubmit(this);");
page_menu_space(out);
page_menu_end(out);
}
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>