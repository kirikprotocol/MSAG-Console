<%@
 include file="/WEB-INF/inc/code_header.jsp"%><%@
 page import="ru.novosoft.smsc.jsp.smsc.reshedule.Edit,
              java.net.URLEncoder,
              ru.novosoft.smsc.jsp.SMSCJspException,
              ru.novosoft.smsc.jsp.SMSCErrors"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.reshedule.Edit"
/><jsp:setProperty name="bean" property="*"/><%
//ServiceIDForShowStatus = Constants.SMSC_SME_ID;
FORM_METHOD = "POST";
TITLE = "SMSC Rescheduling";
MENU0_SELECTION = "MENU0_SMSC_Reshedule";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case Edit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Edit.RESULT_OK:
		STATUS.append("Ok");
		break;
	case Edit.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%@
include file="/WEB-INF/inc/html_3_header.jsp"%><%@
include file="/WEB-INF/inc/collapsing_tree.jsp"%><%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Done",  "Save config");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMSC Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=oldShedule value="<%=bean.getOldShedule()%>">
<input type=hidden name=defaultReshedule value="<%=bean.isDefaultReshedule()%>">
<%@ include file="body.jsp"%>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Done",  "Save config");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@
 include file="/WEB-INF/inc/html_3_footer.jsp"%><%@
 include file="/WEB-INF/inc/code_footer.jsp"%>