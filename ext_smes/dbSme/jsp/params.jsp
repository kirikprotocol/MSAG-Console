<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.dbsme.Params,
                 ru.novosoft.smsc.jsp.PageBean"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.dbsme.Params" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = Constants.WSME_SME_ID;
	TITLE="DB SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.RESULT_OK;
	switch(beanResult = bean.process(request))
	{
    case PageBean.RESULT_OK:
    case PageBean.RESULT_ERROR:
      break;
		case Params.RESULT_DONE:
		case Params.RESULT_OVERVIEW:
			response.sendRedirect("index.jsp");
			return;
		case Params.RESULT_PARAMS:
		case Params.RESULT_DRIVERS:
		case Params.RESULT_PROVIDERS:
		case Params.RESULT_PROVIDER:
			response.sendRedirect(bean.getMenuSelection() + ".jsp");
			return;
		default:
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<%@ include file="inc/menu.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<%
	startSection(out, "DBSme", "DBSme", true);
		startParams(out);
			param(out, "SvcType",     "svcType",     bean.getSvcType());
			param(out, "ProtocolId",  "protocolId",  bean.getProtocolId());
			param(out, "origAddress", "origAddress", bean.getOrigAddress(), null, null, false, "mask");
			param(out, "systemType",  "systemType",  bean.getSystemType());
		finishParams(out);
    startSection(out, "DBSme.Admin", "Admin", false);
      startParams(out);
        param(out, "adminHost", "adminHost", bean.getAdminHost(), null, null, false, "nonEmpty");
        param(out, "adminPort", "adminPort", bean.getAdminPort(), null, null, false, "port");
      finishParams(out);
    finishSection(out);
		startSection(out, "DBSme.ThreadPool", "ThreadPool", false);
			startParams(out);
				param(out, "max",  "max",  bean.getMax(), null, null, false, "positive");
				param(out, "init", "init", bean.getInit(), null, null, false, "positive");
			finishParams(out);
		finishSection(out);
		startSection(out, "DBSme.SMSC", "SMSC", false);
			startParams(out);
				param(out, "host",     "host",     bean.getHost(), null, null, false, "nonEmpty");
				param(out, "port",     "port",     bean.getPort(), null, null, false, "port");
				param(out, "sid",      "sid",      bean.getSid());
				param(out, "timeout",  "timeout",  bean.getTimeout(), null, null, false, "positive");
				param(out, "password", "password", bean.getPassword());
			finishParams(out);
		finishSection(out);
		startSection(out, "DBSme.MessageSet", "Global messages", false);
			startParams(out);
				param(out, "PROVIDER_NOT_FOUND", "provider_not_found", bean.getProvider_not_found(), true);
				param(out, "JOB_NOT_FOUND",      "job_not_found",      bean.getJob_not_found(),      true);
				param(out, "DS_FAILURE",         "ds_failure",         bean.getDs_failure(),         true);
				param(out, "DS_CONNECTION_LOST", "ds_connection_lost", bean.getDs_connection_lost(), true);
				param(out, "DS_STATEMENT_FAIL",  "ds_statement_fail",  bean.getDs_statement_fail(),  true);
				param(out, "QUERY_NULL",         "query_null",         bean.getQuery_null(),         true);
				param(out, "INPUT_PARSE",        "input_parse",        bean.getInput_parse(),        true);
				param(out, "OUTPUT_FORMAT",      "output_format",      bean.getOutput_format(),      true);
				param(out, "INVALID_CONFIG",     "invalid_config",     bean.getInvalid_config(),     true);
			finishParams(out);
		finishSection(out);
	finishSection(out);
%>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbDone",  "Done",  "");
page_menu_button(out, "mbCancel", "Cancel", "Cancel changes", "return noValidationSubmit(this);");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>