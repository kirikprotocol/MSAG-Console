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

	int beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal);
	switch(beanResult)
	{
		case TaskEdit.RESULT_DONE:
			response.sendRedirect("tasks.jsp");
			return;
		default:
      {
        %><%@ include file="inc/menu_switch.jsp"%><%
      }
	}
  int rowN = 0;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="/WEB-INF/inc/time.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=create value=<%=bean.isCreate()%>>
<input type=hidden name=oldTask value="<%=bean.getOldTask()%>">
<table class=properties_list>
<col width="10%">
<tr class=row<%=rowN++&1%>>
  <th>Task ID</th>
  <td><input class=txt name=sectionName value="<%=StringEncoderDecoder.encode(bean.getSectionName())%>"></td>
</tr>
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
  <th><label for=enabled>Enabled</label></th>
  <td><input class=check type=checkbox id=enabled name=enabled value=true <%=bean.isEnabled() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=transactionMode>transaction mode</label></th>
  <td><input class=check type=checkbox id=transactionMode name=transactionMode value=true <%=bean.isTransactionMode() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Priority</th>
  <td><input class=txt name=priority value="<%=StringEncoderDecoder.encode(bean.getPriority())%>" validation="int_range" range_min="1" range_max="100" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Active period</th>
  <td>
    <input class=timeField id=activePeriodStart name=activePeriodStart value="<%=StringEncoderDecoder.encode(bean.getActivePeriodStart())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodStart, false, true);">...</button>
    to
    <input class=timeField id=activePeriodEnd name=activePeriodEnd value="<%=StringEncoderDecoder.encode(bean.getActivePeriodEnd())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodEnd, false, true);">...</button>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Validity period or date</th>
  <td>
    <input class=timeField id=validityPeriod name=validityPeriod value="<%=StringEncoderDecoder.encode(bean.getValidityPeriod())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(validityPeriod, false, true);">...</button>
    or
    <input class=calendarField id=validityDate name=validityDate value="<%=StringEncoderDecoder.encode(bean.getValidityDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(validityDate, false, true);">...</button>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>End date</th>
  <td nowrap><input class=calendarField id=endDate name=endDate value="<%=StringEncoderDecoder.encode(bean.getEndDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(endDate, false, true);">...</button></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Query</th>
  <td><textarea name=query><%=StringEncoderDecoder.encode(bean.getQuery())%></textarea></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Template</th>
  <td><textarea name=template><%=StringEncoderDecoder.encode(bean.getTemplate())%></textarea></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=retryOnFail>Retry on fail (time)</label></th>
  <td>
    <input class=check type=checkbox id=retryOnFail name=retryOnFail value=true <%=bean.isRetryOnFail() ? "checked" : ""%> onClick="opForm.all.retryTime.disabled = !this.checked;">
    <input class=txt name=retryTime id=retryTime value="<%=StringEncoderDecoder.encode(bean.getRetryTime())%>">
    <script>opForm.all.retryTime.disabled = !opForm.all.retryOnFail.checked;</script>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=replaceMessage>Replace message (service type)</label></th>
  <td>
    <input class=check type=checkbox id=replaceMessage name=replaceMessage value=true <%=bean.isReplaceMessage() ? "checked" : ""%> onClick="opForm.all.svcType.disabled = !this.checked;">
    <input class=txt id=svcType name=svcType value="<%=StringEncoderDecoder.encode(bean.getSvcType())%>">
    <script>opForm.all.svcType.disabled = !opForm.all.replaceMessage.checked;</script>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>dsTimeout</th>
  <td><input class=txt name=dsOwnTimeout value="<%=StringEncoderDecoder.encode(bean.getDsTimeout())%>">secs</td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>messages cache size </th>
  <td><input class=txt name=messagesCacheSize value="<%=StringEncoderDecoder.encode(bean.getMessagesCacheSize())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>messages cache sleep</th>
  <td><input class=txt name=messagesCacheSleep value="<%=StringEncoderDecoder.encode(bean.getMessagesCacheSleep())%>">secs</td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>uncommited in generation</th>
  <td><input class=txt name=uncommitedInGeneration value="<%=StringEncoderDecoder.encode(bean.getUncommitedInGeneration())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>uncommited in process</th>
  <td><input class=txt name=uncommitedInProcess value="<%=StringEncoderDecoder.encode(bean.getUncommitedInProcess())%>"></td>
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