<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.util.config.Config,
					  java.util.Set,
					  java.util.Iterator,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  ru.novosoft.smsc.dbsme.Job,
					  ru.novosoft.smsc.jsp.util.tables.DataItem,
					  java.net.URLEncoder,
					  ru.novosoft.smsc.dbsme.Provider,
					  java.util.List,
					  java.util.LinkedList,
                 ru.novosoft.smsc.jsp.PageBean"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.dbsme.Job" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("dbsme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.RESULT_OK;
	switch(beanResult = bean.process(request))
	{
    case PageBean.RESULT_OK:
    case PageBean.RESULT_ERROR:
      break;
/*		case Job.RESULT_EDIT:
			response.sendRedirect("job.jsp?providerName=" + URLEncoder.encode(bean.getProviderName())
										 + "&jobName=" + URLEncoder.encode(bean.getJobName()));
			return;
*/
    case Job.RESULT_DONE:
			response.sendRedirect("provider.jsp?providerName=" + bean.getProviderName());
			return;
		case Job.RESULT_OVERVIEW:
			response.sendRedirect("index.jsp");
			return;
		case Job.RESULT_PARAMS:
		case Job.RESULT_DRIVERS:
		case Job.RESULT_PROVIDERS:
		case Job.RESULT_PROVIDER:
			response.sendRedirect(bean.getMenuSelection() + ".jsp");
			return;
		default:
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/menu.jsp"%>
<div class=content>
<%@include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<input type=hidden name=providerName value="<%=StringEncoderDecoder.encode(bean.getProviderName())%>">
<input type=hidden name=jobId value="<%=StringEncoderDecoder.encode(bean.getJobId())%>">
<input type=hidden name=creating value="<%=bean.isCreating()%>">
<input type=hidden name=initialized value=true>
<script>
function setType()
{
	switch(opForm.type.selectedIndex)
	{
	   case 0:
			ROW_TYPE_SQL1.runtimeStyle.display = "block";
			ROW_TYPE_PLSQL1.runtimeStyle.display = "none";
			ROW_TYPE_PLSQL2.runtimeStyle.display = "none";
			break;
		case 1:
			ROW_TYPE_SQL1.runtimeStyle.display = "none";
			ROW_TYPE_PLSQL1.runtimeStyle.display = "block";
			ROW_TYPE_PLSQL2.runtimeStyle.display = "block";
			break;
	}
}
</script>
<div class=page_subtitle><%=StringEncoderDecoder.encode(bean.getProviderName() + " : " + (bean.isCreating() ? getLocString("dbsme.label.new_job") : bean.getJobName()))%></div>
<%
	List types = new LinkedList();
	types.add(Job.TYPE_SQL);
	types.add(Job.TYPE_PLSQL);
	startSection(out, "job_properties", "Properties", true);
		startParams(out);
			param(out, "name", "jobName", bean.getJobName());
			paramSelect(out, "type", "type", types, bean.getType(), "setType();");
			param(out, "address", "address", bean.getAddress(), null, null, false, "mask");
			param(out, "alias", "alias", bean.getAlias(), null, null, false, "mask");
			if (bean.isProviderWatchdog())
				param(out, "timeout", "timeout", bean.getTimeout(), null, null, false, "positive");
			param(out, "query", "query", bean.isQuery(), "ROW_TYPE_SQL1");
			param(out, "commit", "commit", bean.isCommit(), "ROW_TYPE_PLSQL1");
			param(out, "function", "function", bean.isFunction(), "ROW_TYPE_PLSQL2");
		finishParams(out);
	finishSection(out);
	startSection(out, "Queries", "Queries", true);
		startParams(out);
			paramTextarea(out, "sql", "sql", bean.getSql());
			paramTextarea(out, "input", "input", bean.getInput());
			paramTextarea(out, "output", "output", bean.getOutput());
		finishParams(out);
	finishSection(out);
	startSection(out, "Messages", "Messages", false);
		startParams(out);
			param(out, "DS_FAILURE", "ds_failure", bean.getDs_failure());
			param(out, "DS_CONNECTION_LOST", "ds_connection_lost", bean.getDs_connection_lost());
			param(out, "DS_STATEMENT_FAIL", "ds_statement_fail", bean.getDs_statement_fail());
			param(out, "QUERY_NULL", "query_null", bean.getQuery_null());
			param(out, "INPUT_PARSE", "input_parse", bean.getInput_parse());
			param(out, "OUTPUT_FORMAT", "output_format", bean.getOutput_format());
			param(out, "INVALID_CONFIG", "invalid_config", bean.getInvalid_config());
		finishParams(out);
	finishSection(out);
%>
<script>
setType();
</script>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",   "dbsme.hint.accept");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "dbsme.hint.cancel", "return noValidationSubmit(this);");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>