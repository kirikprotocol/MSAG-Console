<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.dbsme.Index,
					  java.util.Iterator,
					  ru.novosoft.smsc.util.SortedList"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.dbsme.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
	ServiceIDForShowStatus = Constants.DBSME_SME_ID;
	TITLE="DB SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.RESULT_OK;
	switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
	{
		case Index.RESULT_OK:
			STATUS.append("Ok");
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
		case Index.RESULT_ERROR:
			STATUS.append("<span class=CF00>Error</span>");
			break;
		default:
			STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<%@ include file="inc/menu.jsp"%>
<div class=content>
<div class=page_subtitle>DB SME Config <%=bean.isConfigChanged() ? "changed" : "not changed"%></div>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbApply", "Save", "Save new config");
page_menu_button(out, "mbReset", "Reset", "Cancel changes");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>