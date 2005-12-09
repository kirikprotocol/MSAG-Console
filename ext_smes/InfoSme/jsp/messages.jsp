<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.*, ru.novosoft.smsc.infosme.backend.Message,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataItem"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Messages" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

  int rowN = 0;
  int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="inc/header.jsp"%>
<script type="text/javascript">
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
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=sort>
<%Collection allTasks = bean.getAllTasks();
  if (allTasks.size() > 0) {%>
<table class=properties_list>
<col width="10%">
<col width="40%">
<col width="10%">
<col width="40%">
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left">Task:</th>
  <td><select name=taskId><%
  for (Iterator i = allTasks.iterator(); i.hasNext();) {
    String taskId = (String) i.next();
    String taskIdEnc = StringEncoderDecoder.encode(taskId);
  %><option value="<%=taskIdEnc%>" <%= (bean.isTaskId(taskIdEnc)) ? "selected":""%>><%=StringEncoderDecoder.encode(bean.getTaskName(taskId))%></option>
<%}%></select></td>
  <th style="text-align:left"><%= getLocString("infosme.label.msg_state")%></th>
  <td><select name=status>
    <option value="<%=Message.MESSAGE_UNDEFINED_STATE%>" <%= (bean.isStatus(Message.MESSAGE_UNDEFINED_STATE)) ? "selected":""%>>ALL</option>
    <option value="<%=Message.MESSAGE_NEW_STATE%>"       <%= (bean.isStatus(Message.MESSAGE_NEW_STATE)) ? "selected":""%>      >NEW</option>
    <option value="<%=Message.MESSAGE_WAIT_STATE%>"      <%= (bean.isStatus(Message.MESSAGE_WAIT_STATE)) ? "selected":""%>     >WAIT</option>
    <option value="<%=Message.MESSAGE_ENROUTE_STATE%>"   <%= (bean.isStatus(Message.MESSAGE_ENROUTE_STATE)) ? "selected":""%>  >ENROUTE</option>
    <option value="<%=Message.MESSAGE_DELIVERED_STATE%>" <%= (bean.isStatus(Message.MESSAGE_DELIVERED_STATE)) ? "selected":""%>>DELIVERED</option>
    <option value="<%=Message.MESSAGE_EXPIRED_STATE%>"   <%= (bean.isStatus(Message.MESSAGE_EXPIRED_STATE)) ? "selected":""%>  >EXPIRED</option>
    <option value="<%=Message.MESSAGE_FAILED_STATE%>"    <%= (bean.isStatus(Message.MESSAGE_FAILED_STATE)) ? "selected":""%>   >FAILED</option>
  </select></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left"><%= getLocString("infosme.label.from_date")%></th>
  <td><input class=calendarField id=fromDate name=fromDate value="<%=StringEncoderDecoder.encode(bean.getFromDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
  <th style="text-align:left"><%= getLocString("infosme.label.till_date")%></th>
  <td><input class=calendarField id=tillDate name=tillDate value="<%=StringEncoderDecoder.encode(bean.getTillDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left"><%= getLocString("infosme.label.for_abonent")%></th>
  <td><input class=txt type="text" id="address" name="address"  value="<%=bean.getAddress()%>" size=25 maxlength=25></td>
  <th><%= getLocString("infosme.label.page_size")%></th>
  <td><select name=pageSize>
    <option value=  "5" <%= (bean.getPageSizeInt() ==   5) ? "selected":""%>  >5</option>
    <option value= "10" <%= (bean.getPageSizeInt() ==  10) ? "selected":""%> >10</option>
    <option value= "15" <%= (bean.getPageSizeInt() ==  15) ? "selected":""%> >15</option>
    <option value= "20" <%= (bean.getPageSizeInt() ==  20) ? "selected":""%> >20</option>
    <option value= "25" <%= (bean.getPageSizeInt() ==  25) ? "selected":""%> >25</option>
    <option value= "50" <%= (bean.getPageSizeInt() ==  50) ? "selected":""%> >50</option>
    <option value="100" <%= (bean.getPageSizeInt() == 100) ? "selected":""%>>100</option>
  </select></td>
</tr>
</table>
<%}%>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "infosme.hint.query_messages");
page_menu_space(out);
page_menu_end(out);
if (bean.isInitialized()) {
%><div class=content><%
QueryResultSet allMessages = bean.getMessages();
if (allMessages == null || allMessages.getTotalSize() == 0)
{%><span style='color:blue;'><%= getLocString("infosme.msg.no_messages_matched")%></span><%}
else
{%>
<div class=page_subtitle><%= getLocString("infosme.label.query_results_b")%>&nbsp;<%= bean.getTotalSize()%>&nbsp;<%= getLocString("infosme.label.query_results_e")%></div>
<table class=list cellspacing=0>
<col width="1%">
<col width="10%">
<col width="15%">
<col width="13%">
<col width="22%">
<col width="40%">
<thead><tr class=row<%=rowN++&1%>>
  <th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
  <th style="text-align:left">ID</th>
  <th style="text-align:left"><a href="#" <%=bean.getSort().endsWith(Message.SORT_BY_ABONENT) ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("infosme.label.sort_abonent")%>" onclick='return setSort("<%= Message.SORT_BY_ABONENT%>")'><%= getLocString("infosme.label.abonent")%></a></th>
  <th style="text-align:left"><a href="#" <%=bean.getSort().endsWith(Message.SORT_BY_STATUS)  ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("infosme.label.sort_msg_status")%>" onclick='return setSort("<%= Message.SORT_BY_STATUS%>")'><%= getLocString("infosme.label.msg_status")%></a></th>
  <th style="text-align:left"><a href="#" <%=bean.getSort().endsWith(Message.SORT_BY_DATE)    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("infosme.label.sort_date")%>" onclick='return setSort("<%= Message.SORT_BY_DATE%>")'><%= getLocString("infosme.label.date")%></a></th>
  <th style="text-align:left"><%= getLocString("infosme.label.message")%></th>
</tr></thead>
<tbody><%
  for (Iterator i=allMessages.iterator(); i.hasNext();) {
    MessageDataItem message = (MessageDataItem)i.next();
    %><tr class=row<%=rowN++&1%>>
      <td><input class=check type=checkbox name=checked value="<%=message.getId()%>" <%=bean.isMessageChecked(message.getIdString()) ? "checked" : ""%>></td>
      <td><%=StringEncoderDecoder.encode(message.getIdString())%></td>
      <td><%=StringEncoderDecoder.encode(message.getAbonent())%></td>
      <td><%=StringEncoderDecoder.encode(message.getStatus())%></td>
      <td nowrap><%=StringEncoderDecoder.encode(message.getDate())%></td>
      <td><%=StringEncoderDecoder.encode(message.getMessage())%></td>
    </tr>
<%}%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbDelete",    "infosme.button.delete_msg", "infosme.hint.delete_msg");
page_menu_button(session, out, "mbResend",    "infosme.button.resend_msg", "infosme.hint.resend_msg");
page_menu_space(out);
page_menu_button(session, out, "mbDeleteAll", "infosme.button.delete_all", "infosme.hint.delete_all");
page_menu_button(session, out, "mbResendAll", "infosme.button.resend_all", "infosme.hint.resend_all");
page_menu_end(out);
}}%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>