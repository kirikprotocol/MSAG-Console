<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.dbsme.Index,
					  java.util.Iterator,
					  ru.novosoft.smsc.util.SortedList,
                 ru.novosoft.smsc.jsp.PageBean"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.dbsme.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
	ServiceIDForShowStatus = Constants.DBSME_SME_ID;
	TITLE="DB SME Administration";
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
 <%=bean.isApplyAll() ? "checked" : ""%>><label for=applyAllCheckbox>Apply all</label></br>
<input class=check type=checkbox name=apply id=applyJobCheckbox value=jobs
 <%=bean.isJobsChanged() && !bean.isConfigChanged() ? "" : "disabled"%>
 <%=bean.isApplyJobs() || bean.isConfigChanged() ? "checked" : ""%>><label for=applyJobCheckbox>Apply jobs</label>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbApply", "Apply", "Apply new config");
page_menu_button(out, "mbReset", "Reset", "Cancel changes");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>