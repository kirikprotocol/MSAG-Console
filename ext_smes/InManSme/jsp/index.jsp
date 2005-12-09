<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.sibinco.inman.beans.Index, ru.novosoft.smsc.util.*,
                 ru.novosoft.smsc.jsp.SMSCJspException, ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.admin.service.ServiceInfo"%>
<jsp:useBean id="inManBean" scope="page" class="ru.sibinco.inman.beans.Index"/><%
  Index bean = inManBean;
%><jsp:setProperty name="inManBean" property="*" /><%
  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
  FORM_METHOD = "POST";
  TITLE = getLocString("inman.title");
  MENU0_SELECTION = "MENU0_INMANSME";
  switch(bean.process(request))
  {
    case Index.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case Index.RESULT_OK:
    case Index.RESULT_ERROR:
      break;
    default:
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
  int rowN=0;
%>
<%@include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbApply", "common.buttons.apply", "common.buttons.apply");
  page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset", "clickCancel()");
  page_menu_space(out);
  page_menu_button(session, out, "mbStart", "common.buttons.start", "inman.startService", bean.getStatus() == ServiceInfo.STATUS_STOPPED);
  page_menu_button(session, out, "mbStop",  "common.buttons.stop",  "inman.stopService",  bean.getStatus() == ServiceInfo.STATUS_RUNNING);
  page_menu_end(out);
%>
<script type="text/javascript" language="JavaScript">
function refreshStartStopButtonsStatus()
{
    var status = document.getElementById('RUNNING_STATUSERVICE_<%=ServiceIDForShowStatus%>').innerText;
    document.getElementById('mbStart').disabled = (status == "<%= getLocString("common.statuses.online1") %>" ||
                                     status == "<%= getLocString("common.statuses.online2") %>" ||
                                     status == "<%= getLocString("common.statuses.running") %>" ||
                                     status == "<%= getLocString("common.statuses.stopping")%>" ||
                                     status == "<%= getLocString("common.statuses.starting")%>" ||
                                     status == "<%= getLocString("common.statuses.unknown" )%>" );
    document.getElementById('mbStop').disabled  = (status == "<%= getLocString("common.statuses.offline") %>" ||
                                     status == "<%= getLocString("common.statuses.stopped") %>" ||
                                     status == "<%= getLocString("common.statuses.stopping")%>" ||
                                     status == "<%= getLocString("common.statuses.unknown") %>" );
	window.setTimeout(refreshStartStopButtonsStatus, 500);
}
refreshStartStopButtonsStatus();
</script>
<%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ InMan Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=content>
<div class=secSmsc><%=getLocString("inman.paramsSubTitle")%></div>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0>
<tr class=row<%= (rowN++)%2%>>
	<th>SSF Address</th>
	<td nowrap><input class=txt type=text name=ssfAddress value="<%=bean.getSsfAddress()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
    <th>SSN</th>
    <td><input class=txt name=ssn maxlength="5" value="<%=bean.getSsn()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>SCF Address</th>
	<td nowrap><input class=txt type=text name=scfAddress value="<%=bean.getScfAddress()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>Host</th>
	<td nowrap><input class=txt name=host value="<%=bean.getHost()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>Port</th>
	<td nowrap><input class=txt name=port maxlength="5" value="<%=bean.getPort()%>" validation="port"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>CDR mode</th>
    <td><div class=select>
		<select name="cdrMode">
		<option value="all"<%=bean.getCdrMode().equals("all") ? " selected" : ""%>><%=getLocString("inman.cdr.all")%></option>
        <option value="ussd"<%=bean.getCdrMode().equals("ussd") ? " selected" : ""%>><%=getLocString("inman.cdr.ussd")%></option>
        <option value="sms"<%=bean.getCdrMode().equals("sms") ? " selected" : ""%>><%=getLocString("inman.cdr.sms")%></option>
		</select></div>
    </td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>Bill mode</th>
    <td><div class=select>
		<select name="billMode">
		<option value="none"<%=bean.getBillMode().equals("none") ? " selected" : ""%>><%=getLocString("inman.bill.none")%></option>
        <option value="all"<%=bean.getBillMode().equals("all") ? " selected" : ""%>><%=getLocString("inman.bill.all")%></option>
        <option value="postpaid"<%=bean.getBillMode().equals("postpaid") ? " selected" : ""%>><%=getLocString("inman.bill.postpaid")%></option>
        <option value="prepaid"<%=bean.getBillMode().equals("prepaid") ? " selected" : ""%>><%=getLocString("inman.bill.prepaid")%></option>
		</select></div>
    </td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>Billing dir</th>
	<td nowrap><input class=txt type=text name="billingDir" value="<%=StringEncoderDecoder.encode(bean.getBillingDir())%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>Billing interval (sec)</th>
    <td><input class=txt name=billingInterval maxlength="5" value="<%=bean.getBillingInterval()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
    <th>IN_Timeout (sec)</th>
    <td><input class=txt name=inTimeout maxlength="5" value="<%=bean.getInTimeout()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
    <th>SMSC_Timeout (sec)</th>
    <td><input class=txt name=smscTimeout maxlength="5" value="<%=bean.getSmscTimeout()%>"></td>
</tr>
</table>
</div><%
  page_menu_begin(out);
  page_menu_button(session, out, "mbApply", "common.buttons.apply", "common.buttons.apply");
  page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
