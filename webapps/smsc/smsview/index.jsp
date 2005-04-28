<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.smsview.*,
                 ru.novosoft.smsc.util.*,
                 java.text.*,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 java.util.Collection,
                 java.util.Iterator"%>
<%@ page import="ru.novosoft.smsc.jsp.smsview.*"%>
<jsp:useBean id="smsViewBean" scope="session" class="ru.novosoft.smsc.jsp.smsview.SmsViewFormBean" />
<%
  SmsViewFormBean bean = smsViewBean;
  if( request.getParameter("mbQuery") != null)
    bean.refreshQuery();
  bean.setExactRowsCount(false);
%>
<jsp:setProperty name="smsViewBean" property="*"/>
<%
  TITLE="SMS View";
  MENU0_SELECTION = "MENU0_SMSVIEW";

  int beanResult = bean.process(request);
  switch(beanResult)
  {
    case SmsViewFormBean.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case SmsViewFormBean.RESULT_FILTER:
    case SmsViewFormBean.RESULT_OK:
      break;
    case SmsViewFormBean.RESULT_ERROR:
      break;
    default:
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content><%
{%>
<div class=page_subtitle>Search parameters</div>
<table class=properties_list cell>
<tr class=row0>
  <th>Storage:</th>
  <td nowrap><input
      class=radio type="radio" name="storageType" id=storageTypeArchive value="<%=
        SmsQuery.SMS_ARCHIVE_STORAGE_TYPE%>"   <%=
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
  <th>Abonent Address:</th>
  <td><input class=txt type="text" id="abonentAddress" name="abonentAddress" value="<%=bean.getAbonentAddress()%>" size=25 maxlength=25 onkeyup="hideAddresses()"></td>
  <th>SME Id:</th>
  <td><input class=txt type="text" id="smeId" name="smeId" value="<%=bean.getSmeId()%>" size=17 maxlength=15 onkeyup="hideSmeIds()"></td>
</tr>
<tr class=row0>
  <th>Source Address:</th>
  <td><input class=txt type="text" id="fromAddress" name="fromAddress"  value="<%=bean.getFromAddress()%>" size=25 maxlength=25></td>
  <th>Source SME Id:</th>
  <td><input class=txt type="text" id="srcSmeId" name="srcSmeId"  value="<%=bean.getSrcSmeId()%>" size=17 maxlength=15></td>
</tr>
<tr class=row1>
  <th>Destination Address:</th>
  <td><input class=txt type="text" id="toAddress" name="toAddress" value="<%=bean.getToAddress()%>" size=25 maxlength=25></td>
  <th>Destination SME Id:</th>
  <td><input class=txt type="text" id="dstSmeId" name="dstSmeId"  value="<%=bean.getDstSmeId()%>" size=17 maxlength=15></td>
</tr>
<tr class=row0>
  <th>SMS Id:</th>
  <td><input class=txt type="text" name="smsId"  value="<%=bean.getSmsId()%>" size=16 maxlength=16></td>
  <th>Route Id:</th>
  <td><input class=txt type="text" name="routeId"  value="<%=bean.getRouteId()%>" size=32 maxlength=32></td>
</tr>
<tr class=row1>
  <th>SMS Status:</th>
  <td nowrap>
  <select name="status"><%int status=bean.getStatus();%>
    <option value="-1" <%= (status == SmsQuery.SMS_UNDEFINED_VALUE) ? "selected":""%>>ALL</option>
    <option value="0"  <%= (status == 0) ? "selected":""%>>ENROUTE</option>
    <option value="1"  <%= (status == 1) ? "selected":""%>>DELIVERED</option>
    <option value="2"  <%= (status == 2) ? "selected":""%>>EXPIRED</option>
    <option value="3"  <%= (status == 3) ? "selected":""%>>UNDELIVERABLE</option>
    <option value="4"  <%= (status == 4) ? "selected":""%>>DELETED</option>
  </select>
  </td>
  <th>Last result:</th>
  <td nowrap>
  <select name="lastResult"><%int lastResult=bean.getLastResult();%>
    <option value="-1" <%= (lastResult == SmsQuery.SMS_UNDEFINED_VALUE) ? "selected":""%>>ALL</option>
    <%Collection errorValues = bean.getErrorValues();
      if (errorValues != null) {
        for (Iterator it=errorValues.iterator(); it.hasNext(); ) {
          Object obj = it.next();
          if (obj == null || !(obj instanceof SmsViewFormBean.ErrorValue)) continue;
          SmsViewFormBean.ErrorValue errorValue = (SmsViewFormBean.ErrorValue)obj;
          %><option value="<%= errorValue.errorCode%>" <%= (lastResult == errorValue.errorCode) ? "selected":""%>><%=
                    errorValue.errorString+" ("+errorValue.errorCode+")"%></option><%
        }
      }%>
  </select>
  </td>
</tr>
<tr class=row0>
  <th>From Date:</th>
  <td nowrap><input type=text id=fromDate name=fromDate class=calendarField value="<%=bean.getFromDate()%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
  <th>Till Date:</th>
  <td nowrap><input type=text id="tillDate" name="tillDate" class=calendarField value="<%=bean.getTillDate()%>" maxlength=20><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
<tr class=row1>
  <th>Rows maximum / Per page:</th>
  <td nowrap>
  <select name="rowsMaximum"><%int rowsMaximum = bean.getRowsMaximum();%>
		<option value="100" <%= (rowsMaximum <= 100) ?   "selected":""%>>100</option>
		<option value="200" <%= (rowsMaximum == 200) ?   "selected":""%>>200</option>
		<option value="300" <%= (rowsMaximum == 300) ?   "selected":""%>>300</option>
		<option value="400" <%= (rowsMaximum == 400) ?   "selected":""%>>400</option>
		<option value="500" <%= (rowsMaximum == 500) ?   "selected":""%>>500</option>
		<option value="1000" <%= (rowsMaximum == 1000) ? "selected":""%>>1000</option>
		<option value="2000" <%= (rowsMaximum == 2000) ? "selected":""%>>2000</option>
		<option value="5000" <%= (rowsMaximum >= 5000) ? "selected":""%>>5000</option>
	</select>&nbsp;/&nbsp;<select name="pageSize"><%int rowsToDisplay = bean.getPageSizeInt();%>
		<option value="5" <%= (rowsToDisplay < 10) ?   "selected":""%>>5</option>
		<option value="10" <%= (rowsToDisplay == 10) ? "selected":""%>>10</option>
		<option value="20" <%= (rowsToDisplay == 20) ? "selected":""%>>20</option>
		<option value="30" <%= (rowsToDisplay == 30) ? "selected":""%>>30</option>
		<option value="40" <%= (rowsToDisplay == 40) ? "selected":""%>>40</option>
		<option value="50" <%= (rowsToDisplay == 50) ? "selected":""%>>50</option>
		<option value="-1" <%= (rowsToDisplay < 0 || rowsToDisplay > 50) ?  "selected":""%>>All</option>
	</select>
  </td>
  <td nowrap>
    <input class=check type=checkbox name=exactRowsCount id=exactRowsCount <%=bean.isExactRowsCount() ? "checked" : ""%>>
    <label for=exactRowsCount>Query exact rows count</label>
  </td>
</tr>
</table>
<script>
function hideAddresses()
{
  if (document.all.abonentAddress.value != "" && document.all.abonentAddress.value != "*") {
    document.all.fromAddress.disabled = document.all.toAddress.disabled = true;
    document.all.fromAddress.value    = document.all.toAddress.value    = "*";
  } else {
    document.all.fromAddress.disabled = document.all.toAddress.disabled = false;
  }
  return true;
}
function hideSmeIds()
{
  if (document.all.smeId.value != "" && document.all.smeId.value != "*") {
    document.all.srcSmeId.disabled = document.all.dstSmeId.disabled = true;
    document.all.srcSmeId.value    = document.all.dstSmeId.value    = "*";
  }
  else {
    document.all.srcSmeId.disabled = document.all.dstSmeId.disabled = false;
  }
  return true;
}

hideAddresses();
hideSmeIds();
</script>
<%}
%></div><%
page_menu_begin(out);
page_menu_button(out, "mbQuery",  "Query !",  "Run query");
page_menu_button(out, "mbClear", "Clear", "Clear query parameters");
page_menu_space(out);
page_menu_end(out);
%><div class=content><%
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
<div class=page_subtitle>Search results: <%= bean.getTotalRowsCount()%> rows fetched by query<%=
    ((!bean.isExactRowsCount() && bean.isHasMore()) ? " (has more rows...)":"")%></div>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
<table class=list cellspacing=0>
<thead>
<tr class=row0>
  <th>&nbsp;</th>
  <th><a href="#" <%=bean.getSort().endsWith("name")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by SMS Id" onclick='return setSort("name")'>ID</a></th>
  <th><a href="#" <%=bean.getSort().endsWith("sendDate") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Submit date/time" onclick='return setSort("sendDate")'>Sms submit/valid</a></th>
  <th><a href="#" <%=bean.getSort().endsWith("lastDate") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Last try date/time" onclick='return setSort("lastDate")'>Tried last/next</a></th>
  <th><a href="#" <%=bean.getSort().endsWith("from")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Source address" onclick='return setSort("from")'>Source</a></th>
  <th><a href="#" <%=bean.getSort().endsWith("to")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Destination address" onclick='return setSort("to")'>Destination</a></th>
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
    row.getBodyParameters();
    long smsRowId = row.getId();
%><tr class=row<%=rowN&1%>0>
      <td nowrap valign=top>
        <input class=check type=checkbox name=checkedRows value="<%= smsRowId%>" <%=bean.isRowChecked(smsRowId) ? "checked" : ""%>>
      </td>
      <td nowrap valign=top style="text-align: right">
        <a target="smsc_smsview" href="<%= CPATH%>/smsview/view.jsp?viewId=<%= smsRowId%>&mbView=show" title="Detailed view"><%= smsRowId%></a><br>
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
            String errMessage = bean.getAppContext().getLocaleString(request.getLocale(), SmsViewFormBean.ERR_CODES_PREFIX+row.getLastResult());
            if (errMessage == null) errMessage = bean.getAppContext().getLocaleString(request.getLocale(), SmsViewFormBean.ERR_CODE_UNKNOWN); %>
        <%= StringEncoderDecoder.encode(errMessage == null ? "" : errMessage)%>
      </td>
  </tr>
  <tr class=row<%=rowN&1%>1>
      <td colspan=8><%= (row.getText()!=null && row.isTextEncoded()) ? row.getText():StringEncoderDecoder.encode(row.getText())%>&nbsp;</td>
  </tr><%
}}
%></tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
<% } %>
</div>
<% if (bean.getTotalSizeInt()>0 && bean.getStorageType() == SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) {
page_menu_begin(out);
page_menu_button(out, "mbRemove", "Delete checked",  "Delete checked messages");
page_menu_button(out, "mbDelete", "Delete All fetched", "Delete All fetched messages");
page_menu_space(out);
page_menu_end(out);
} %>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>