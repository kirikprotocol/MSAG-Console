<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.smsview.*,
                 ru.novosoft.smsc.util.*,
                 java.text.*"%>
<%@ page import="ru.novosoft.smsc.jsp.smsview.*"%>
<jsp:useBean id="smsViewBean" scope="session" class="ru.novosoft.smsc.jsp.smsview.SmsViewFormBean" />
<%
	SmsViewFormBean bean = smsViewBean;
	bean.refreshQuery();
%>
<jsp:setProperty name="smsViewBean" property="*"/>
<%
TITLE="SMS View";
MENU0_SELECTION = "MENU0_SMSVIEW";

int beanResult = SmsViewFormBean.RESULT_OK;
switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case SmsViewFormBean.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case SmsViewFormBean.RESULT_FILTER:
	case SmsViewFormBean.RESULT_OK:
		STATUS.append("Ok");
		break;
	case SmsViewFormBean.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
	default:
		STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%><%
{%>
<div class=secQuestion>Search parameters</div>
<table class=secRep width="100%">
<tr class=row0>
	<th class=label>Storage:</th>
	<td nowrap><input class=radio type="radio" name="storageType" id=storageTypeArchive value="<%=
			SmsQuery.SMS_ARCHIVE_STORAGE_TYPE%>" <%=
			(bean.getStorageType()==SmsQuery.SMS_ARCHIVE_STORAGE_TYPE) ?
			"checked":""%>><label for=storageTypeArchive>Archive</label></td>
	<td nowrap><input
			class=radio type="radio" name="storageType" id=storageTypeOperative value="<%=
			SmsQuery.SMS_OPERATIVE_STORAGE_TYPE%>" <%=
			(bean.getStorageType()==SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) ?
			"checked":""%>><label for=storageTypeOperative>Operative</label>
	</td>
	<td>&nbsp;</td>
	</td>
</tr>
<tr class=row1>
	<th class=label>Source Address:</th>
	<td><input class=txtW type="text" name="fromAddress"  value="<%=bean.getFromAddress()%>" size=25 maxlength=25></td>
	<th class=label>Source SME Id:</th>
	<td><input class=txtW type="text" name="srcSmeId"  value="<%=bean.getSrcSmeId()%>" size=17 maxlength=15></td>
</tr>
<tr class=row0>
	<th class=label>Destination Address:</th>
	<td><input class=txtW type="text" name="toAddress" value="<%=bean.getToAddress()%>" size=25 maxlength=25></td>
	<th class=label>Destination SME Id:</th>
	<td><input class=txtW type="text" name="dstSmeId"  value="<%=bean.getDstSmeId()%>" size=17 maxlength=15></td>
</tr>
<tr class=row1>
	<th class=label>SMS Id:</th>
	<td><input class=txtW type="text" name="smsId"  value="<%=bean.getSmsId()%>" size=16 maxlength=16></td>
	<th class=label>Route Id:</th>
	<td><input class=txtW type="text" name="routeId"  value="<%=bean.getRouteId()%>" size=32 maxlength=32></td>
</tr>
<tr class=row0>
	<th class=label>From Date:</th>
	<td nowrap><input type=text id=fromDate name=fromDate class=calendarField value="<%=bean.getFromDate()%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
	<th class=label>Till Date:</th>
	<td nowrap><input type=text id="tillDate" name="tillDate" class=calendarField value="<%=bean.getTillDate()%>" maxlength=20><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
</table>
<%}%>
<div class=secButtons>Rows max:<select name="rowsMaximum"><%int rowsMaximum = bean.getRowsMaximum();%>
		<option value="100" <%= (rowsMaximum  <= 100) ?   "selected":""%>>100</option>
		<option value="200" <%= (rowsMaximum == 200) ? "selected":""%>>200</option>
		<option value="300" <%= (rowsMaximum == 300) ? "selected":""%>>300</option>
		<option value="400" <%= (rowsMaximum == 400) ? "selected":""%>>400</option>
		<option value="500" <%= (rowsMaximum == 500) ? "selected":""%>>500</option>
		<option value="1000" <%= (rowsMaximum == 1000) ? "selected":""%>>1000</option>
		<option value="2000" <%= (rowsMaximum == 2000) ? "selected":""%>>2000</option>
		<option value="5000" <%= (rowsMaximum >= 5000) ? "selected":""%>>5000</option>
	</select>
Per page:<select name="pageSize"><%int rowsToDisplay = bean.getPageSizeInt();%>
		<option value="5" <%= (rowsToDisplay < 10) ?   "selected":""%>>5</option>
		<option value="10" <%= (rowsToDisplay == 10) ? "selected":""%>>10</option>
		<option value="20" <%= (rowsToDisplay == 20) ? "selected":""%>>20</option>
		<option value="30" <%= (rowsToDisplay == 30) ? "selected":""%>>30</option>
		<option value="40" <%= (rowsToDisplay == 40) ? "selected":""%>>40</option>
		<option value="50" <%= (rowsToDisplay == 50) ? "selected":""%>>50</option>
		<option value="-1" <%= (rowsToDisplay < 0 || rowsToDisplay > 50) ?  "selected":""%>>All</option>
	</select>
<input class=btn type="submit" name="mbQuery" value="Query !">
<input class=btn type="submit" name="mbClear" value="Clear query parameters"></div>
<%
if (bean.getTotalSizeInt()>0) {%>
<input type=hidden name=sort>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<script>
function setSort(sorting)
{
	if (sorting == "<%=bean.getSort()%>")
		opForm.sort.value = "-<%=bean.getSort()%>";
	else
		opForm.sort.value = sorting;

	opForm.submit();
	return false;
}
</script>
<br>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ results ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
  SimpleDateFormat dateFormatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
%>
<div class=secView>Search results: <%= bean.getTotalRowsCount()%> rows matched by query</div>
<%@ include file="/WEB-INF/inc/navbar.jsp"%>
<table class=secRep cellspacing=1 width="100%">
<thead>
<tr class=row0>
  <th>&nbsp;</th>
  <th><a href="#" <%=bean.getSort().endsWith("name")   ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by SMS Id" onclick='return setSort("name")'>ID</a></th>
  <th><a href="#" <%=bean.getSort().endsWith("date")   ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Date" onclick='return setSort("date")'>Date/Valid</a></th>
  <th>Tried last/next</th>
  <th><a href="#" <%=bean.getSort().endsWith("from")   ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Source address" onclick='return setSort("from")'>Source</a></th>
  <th><a href="#" <%=bean.getSort().endsWith("to")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Destination address" onclick='return setSort("to")'>Destination</a></th>
  <th>Route</th>
  <th><a href="#" <%=bean.getSort().endsWith("status") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by delivery status" onclick='return setSort("status")'>Status</a></th>
</tr></thead>
<tbody><%
int firstIndex = bean.getStartPositionInt()+1;
int lastIndex = bean.getStartPositionInt()+bean.getPageSizeInt();
if (lastIndex >= bean.getTotalSizeInt() || bean.getPageSizeInt() < 0)
	lastIndex = bean.getTotalSizeInt();

{
  int rowN=0;
  for (int cnt=firstIndex; cnt<=lastIndex; cnt++, rowN++) {
    SmsRow row = bean.getRow(cnt-1);
    if (row == null) { rowN--; continue; }
    long smsRowId = row.getIdLong();
%><tr class=row<%=rowN&1%>0>
      <td nowrap valign=top>
        <input class=check type=checkbox name=checkedRows value="<%= smsRowId%>" <%=bean.isRowChecked(smsRowId) ? "checked" : ""%>>
      </td>
      <td nowrap valign=top style="text-align: right">
        <a href="view.jsp?smsId=<%= row.getIdString()%>&storageType=<%= bean.getStorageType()%>&mbView=show" title="Detailed view"><%= smsRowId%></a><br>
        <!--%= row.getAttempts()%-->
      </td>
      <td nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
        <%= (row.getSubmitTime()!= null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getSubmitTime())):"&nbsp;"%><br>
        <%= (row.getValidTime()!= null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getValidTime())):"&nbsp;"%>
      </td>
      <td nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
        <%= (row.getLastTryTime()!= null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getLastTryTime())):"&nbsp;"%><br>
        <%= (row.getNextTryTime()!= null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getNextTryTime())):"&nbsp;"%>
      </td>
      <td nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
        <%= StringEncoderDecoder.encode(row.getOriginatingAddressMask().getMask())%><br>
        <%= StringEncoderDecoder.encode(row.getSrcSmeId())%>
      </td>
      <td nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
        <%= StringEncoderDecoder.encode(row.getDestinationAddressMask().getMask())%>
        <%String dea = row.getDealiasedDestinationAddressMask().getMask();
          if( dea != null && dea.trim().length() > 0 && !dea.equals(row.getDestinationAddressMask().getMask())) {
        %>
          (<%=dea%>)
        <%
          }
        %>
        <br>
        <%= StringEncoderDecoder.encode(row.getDstSmeId())%>
      </td>
      <td valign=top nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
        <%= StringEncoderDecoder.encode(row.getRouteId())%>
      </td>
      <td nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
        <%= StringEncoderDecoder.encode(row.getStatus())%> (<%=row.getLastResult()%>) <br>
        <%
            String errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode."+row.getLastResult());
            if (errMessage == null) errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode.unknown"); %>
        <%= StringEncoderDecoder.encode(errMessage)%>
      </td>
  </tr>
  <tr class=row<%=rowN&1%>1>
      <td colspan=8 style="border-top:dotted 1px #C0C0C0;"><%= (row.getText()!=null&&row.getText().startsWith("&#")?row.getText():StringEncoderDecoder.encode(row.getText()))%>&nbsp;</td>
  </tr><%
}}
%></tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar.jsp"%>
<div class=secButtons>
<input class=btn type="submit" name="mbRemove" value="Delete checked rows">
<input class=btn type="submit" name="mbDelete" value="Delete All fetched rows">
</div>
<% } %>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>