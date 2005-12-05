<%@ page import="ru.novosoft.smsc.jsp.SMSCJspException,
				 ru.novosoft.smsc.jsp.SMSCErrors, ru.novosoft.smsc.dbsme.Index,
                 ru.novosoft.smsc.jsp.PageBean, ru.novosoft.smsc.util.Functions"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.dbsme.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
	ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
	TITLE=getLocString("dbsme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.RESULT_OK;
	switch(beanResult = bean.process(request))
	{
    case PageBean.RESULT_OK:
    case PageBean.RESULT_ERROR:
      break;
		case Index.RESULT_DONE:
		case Index.RESULT_OVERVIEW:
			response.sendRedirect("index.jsp");
			return;
		case Index.RESULT_PARAMS:
		case Index.RESULT_DRIVERS:
		case Index.RESULT_PROVIDERS:
		case Index.RESULT_PROVIDER:
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
<input class=check type=checkbox name=apply id=applyAllCheckbox value=all
 <%=bean.isConfigChanged() ? "" : "disabled"%>
 <%=bean.isApplyAll() ? "checked" : ""%>><label for=applyAllCheckbox><%= getLocString("dbsme.label.apply_all")%></label></br>
<input class=check type=checkbox name=apply id=applyJobCheckbox value=jobs
 <%=bean.isJobsChanged() && !bean.isConfigChanged() ? "" : "disabled"%>
 <%=bean.isApplyJobs() || bean.isConfigChanged() ? "checked" : ""%>><label for=applyJobCheckbox><%= getLocString("dbsme.label.apply_jobs")%></label>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbApply", "common.buttons.apply", "dbsme.hint.apply");
page_menu_button(session, out, "mbReset", "common.buttons.reset", "dbsme.hint.reset");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>