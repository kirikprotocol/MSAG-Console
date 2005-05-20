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
  TITLE = getLocString("smsview.title");
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
<div class=page_subtitle><%=getLocString("common.titles.searchParams")%></div>
<table class=properties_list cell>
<tr class=row0>
  <th><%=getLocString("smsview.storage")%>:</th>
  <td nowrap><input
      class=radio type="radio" name="storageType" id=storageTypeArchive value="<%=
        SmsQuery.SMS_ARCHIVE_STORAGE_TYPE%>"   <%=
        (bean.getStorageType()==SmsQuery.SMS_ARCHIVE_STORAGE_TYPE) ?
        "checked":""%>><label for=storageTypeArchive><%=getLocString("smsview.archive")%></label></td>
  <td nowrap><input
      class=radio type="radio" name="storageType" id=storageTypeOperative value="<%=
        SmsQuery.SMS_OPERATIVE_STORAGE_TYPE%>" <%=
        (bean.getStorageType()==SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) ?
        "checked":""%>><label for=storageTypeOperative><%=getLocString("smsview.operative")%></label>
  </td>
  <td>&nbsp;</td>
  </td>
</tr>
<tr class=row1>
  <th><%=getLocString("smsview.abonentAddress")%>:</th>
  <td><input class=txt type="text" id="abonentAddress" name="abonentAddress" value="<%=bean.getAbonentAddress()%>" size=25 maxlength=25 onkeyup="hideAddresses()"></td>
  <th><%=getLocString("smsview.smeId")%>:</th>
  <td><input class=txt type="text" id="smeId" name="smeId" value="<%=bean.getSmeId()%>" size=17 maxlength=15 onkeyup="hideSmeIds()"></td>
</tr>
<tr class=row0>
  <th><%=getLocString("smsview.sourceAddress")%>:</th>
  <td><input class=txt type="text" id="fromAddress" name="fromAddress"  value="<%=bean.getFromAddress()%>" size=25 maxlength=25></td>
  <th><%=getLocString("smsview.sourceSmeId")%>:</th>
  <td><input class=txt type="text" id="srcSmeId" name="srcSmeId"  value="<%=bean.getSrcSmeId()%>" size=17 maxlength=15></td>
</tr>
<tr class=row1>
  <th><%=getLocString("smsview.destAddress")%>:</th>
  <td><input class=txt type="text" id="toAddress" name="toAddress" value="<%=bean.getToAddress()%>" size=25 maxlength=25></td>
  <th><%=getLocString("smsview.destSmeId")%>:</th>
  <td><input class=txt type="text" id="dstSmeId" name="dstSmeId"  value="<%=bean.getDstSmeId()%>" size=17 maxlength=15></td>
</tr>
<tr class=row0>
  <th><%=getLocString("smsview.smsId")%>:</th>
  <td><input class=txt type="text" name="smsId"  value="<%=bean.getSmsId()%>" size=16 maxlength=16></td>
  <th><%=getLocString("smsview.routeId")%>:</th>
  <td><input class=txt type="text" name="routeId"  value="<%=bean.getRouteId()%>" size=32 maxlength=32></td>
</tr>
<tr class=row1>
  <th><%=getLocString("smsview.smsStatus")%>:</th>
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
  <th><%=getLocString("smsview.lastResult")%>:</th>
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
  <th><%=getLocString("common.util.FromDate")%>:</th>
  <td nowrap><input type=text id=fromDate name=fromDate class=calendarField value="<%=bean.getFromDate()%>" maxlength=20 style="z-index:22;">
    <button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button>
  </td>
  <th><%=getLocString("common.util.TillDate")%>:</th>
  <td nowrap><input type=text id="tillDate" name="tillDate" class=calendarField value="<%=bean.getTillDate()%>" maxlength=20>
    <button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button>
  </td>
</tr>
<tr class=row1>
  <th><%=getLocString("smsview.rowsMaxPerPage")%>:</th>
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
    <label for=exactRowsCount><%=getLocString("smsview.queryExactRowsCount")%></label>
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
page_menu_button(session, out, "mbQuery",  "common.buttons.queryExcl",  "common.buttons.runQuery");
page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clearQueryParams");
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
  SimpleDateFormat dateFormatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss", getLoc());
%>
<div class=page_subtitle><%=getLocString("common.titles.searchResultsPre") + bean.getTotalRowsCount() + getLocString("common.titles.searchResultsPost") +
    ((!bean.isExactRowsCount() && bean.isHasMore()) ? getLocString("common.titles.searchResultsPost2"):"")%></div>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
<table class=list cellspacing=0>
<thead>
<tr class=row0>
  <th>&nbsp;</th>
  <th><a href="#" <%=bean.getSort().endsWith("name")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("smsview.idHint")%>"          onclick='return setSort("name")'><%=getLocString("common.sortmodes.id")%></a></th>
  <th><a href="#" <%=bean.getSort().endsWith("sendDate") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("smsview.sortSubmitHint")%>"  onclick='return setSort("sendDate")'><%=getLocString("smsview.sortSubmit")%></a></th>
  <th><a href="#" <%=bean.getSort().endsWith("lastDate") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("smsview.sortTriedHint")%>"   onclick='return setSort("lastDate")'><%=getLocString("smsview.sortTried")%></a></th>
  <th><a href="#" <%=bean.getSort().endsWith("from")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("smsview.sourceHint")%>"      onclick='return setSort("from")'><%=getLocString("common.sortmodes.source")%></a></th>
  <th><a href="#" <%=bean.getSort().endsWith("to")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("smsview.destinationHint")%>" onclick='return setSort("to")'><%=getLocString("common.sortmodes.destination")%></a></th>
  <th><%=getLocString("smsview.route")%></th>
  <th><a href="#" <%=bean.getSort().endsWith("status") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("smsview.statusHint")%>" onclick='return setSort("status")'><%=getLocString("common.sortmodes.status")%></a></th>
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
        <a target="smsc_smsview" href="<%= CPATH%>/smsview/view.jsp?viewId=<%= smsRowId%>&mbView=show" title="<%=getLocString("smsview.viewDetailSubTitle")%>"><%= smsRowId%></a><br>
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
            String errMessage = getLocString(SmsViewFormBean.ERR_CODES_PREFIX+row.getLastResult());
            if (errMessage == null) errMessage = getLocString(SmsViewFormBean.ERR_CODE_UNKNOWN); %>
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
page_menu_button(session, out, "mbRemove", "common.buttons.deleteChecked", "smsview.deleteCheckedHint");
page_menu_button(session, out, "mbDelete", "common.buttons.deleteAllFetched", "smsview.deleteAllFetchedHint");
page_menu_space(out);
page_menu_end(out);
} %>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>