<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants, ru.novosoft.smsc.mcisme.beans.Index,
                 ru.novosoft.smsc.jsp.SMSCJspException, ru.novosoft.smsc.jsp.SMSCErrors,
                 java.util.*, ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.util.Functions,
                 ru.novosoft.smsc.admin.service.ServiceInfo"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
	TITLE="Missed Calls Info SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
  int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<script>
function checkApplyResetButtons()
{
  opForm.all.mbApply.disabled = !(opForm.all.allCheck.checked);
  opForm.all.mbReset.disabled = !((<%=bean.isConfigChanged()%> && opForm.all.allCheck.checked));
}
</script>
<%! private String status(boolean isChanged) { // TODO: ???
  return ((isChanged) ? "<span style='color:red;'>changed</span>":"clear");
}%>
<input type=hidden name=edit>
TODO: Add status string here !!!
</div><%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply changes", bean.isConfigChanged());
page_menu_button(out, "mbReset",  "Reset",  "Discard changes", bean.isConfigChanged());
page_menu_space(out);
page_menu_button(out, "mbStart", "Start", "Start MCI Sme service", bean.getStatus() == ServiceInfo.STATUS_STOPPED);
page_menu_button(out, "mbStop",  "Stop",  "Stop MCI Sme service", bean.getStatus() == ServiceInfo.STATUS_RUNNING);
page_menu_end(out);
%>
<script language="JavaScript">
function refreshStartStopButtonsStatus()
{
	document.all.mbStart.disabled = (document.all.RUNNING_STATUSERVICE_MCISme.innerText != "stopped");
	document.all.mbStop.disabled = (document.all.RUNNING_STATUSERVICE_MCISme.innerText != "running");
	window.setTimeout(refreshStartStopButtonsStatus, 500);
}
refreshStartStopButtonsStatus();
</script>
<div class=content>
TODO: Add page content here !!!
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>