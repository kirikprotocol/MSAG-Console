<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.TaskEdit,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.Iterator"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.TaskEdit" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = Constants.INFO_SME_ID;
	TITLE="Informer SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.RESULT_OK;
	switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
	{
		case TaskEdit.RESULT_APPLY:
			response.sendRedirect("index.jsp");
			return;
		case TaskEdit.RESULT_OPTIONS:
			response.sendRedirect("options.jsp");
			return;
		case TaskEdit.RESULT_DRIVERS:
			response.sendRedirect("drivers.jsp");
			return;
		case TaskEdit.RESULT_PROVIDERS:
			response.sendRedirect("providers.jsp");
			return;
		case TaskEdit.RESULT_TASKS:
			response.sendRedirect("tasks.jsp");
			return;
		case TaskEdit.RESULT_SHEDULES:
			response.sendRedirect("shedules.jsp");
			return;
		case TaskEdit.RESULT_DONE:
			response.sendRedirect("tasks.jsp");
			return;
		case TaskEdit.RESULT_OK:
			STATUS.append("Ok");
			break;
		case TaskEdit.RESULT_ERROR:
			STATUS.append("<span class=CF00>Error</span>");
			break;
		default:
			STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
	}
  int rowN = 0;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=create value=<%=bean.isCreate()%>>
<input type=hidden name=oldTask value="<%=bean.getOldTask()%>">
<table class=properties_list>
<col width="10%">
<tr class=row<%=rowN++&1%>>
  <th>Task name</th>
  <td><input class=txt name=name value="<%=StringEncoderDecoder.encode(bean.getName())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Provider</th>
  <td><select name=provider><%
    for (Iterator i = bean.getAllProviders().iterator(); i.hasNext();) {
      String providerName = (String) i.next();
      String providerNameEnc = StringEncoderDecoder.encode(providerName);
      %><option value="<%=providerNameEnc%>"<%=providerName.equals(bean.getProvider()) ? " selected" : ""%>><%=providerNameEnc%></option><%
    }
  %></select></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Enabled</th>
  <td><input class=check type=checkbox name=enabled value=true <%=bean.isEnabled() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Priority</th>
  <td><input class=txt name=priority value="<%=StringEncoderDecoder.encode(bean.getPriority())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Retry on fail</th>
  <td><input class=check type=checkbox name=retryOnFail value=true <%=bean.isRetryOnFail() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Replace message</th>
  <td><input class=check type=checkbox name=replaceMessage value=true <%=bean.isReplaceMessage() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Service type</th>
  <td><input class=txt name=svcType value="<%=StringEncoderDecoder.encode(bean.getSvcType())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>End date</th>
  <td><input class=txt name=endDate value="<%=StringEncoderDecoder.encode(bean.getEndDate())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Retry time</th>
  <td><input class=txt name=retryTime value="<%=StringEncoderDecoder.encode(bean.getRetryTime())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Validity period</th>
  <td><input class=txt name=validityPeriod value="<%=StringEncoderDecoder.encode(bean.getValidityPeriod())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Validity date</th>
  <td><input class=txt name=validityDate value="<%=StringEncoderDecoder.encode(bean.getValidityDate())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Active period start</th>
  <td><input class=txt name=activePeriodStart value="<%=StringEncoderDecoder.encode(bean.getActivePeriodStart())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Active period end</th>
  <td><input class=txt name=activePeriodEnd value="<%=StringEncoderDecoder.encode(bean.getActivePeriodEnd())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Query</th>
  <td><textarea name=query><%=StringEncoderDecoder.encode(bean.getQuery())%></textarea></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Template</th>
  <td><textarea name=template><%=StringEncoderDecoder.encode(bean.getTemplate())%></textarea></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbDone",  "Done",  "Done editing");
page_menu_button(out, "mbCancel", "Cancel", "Cancel changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>