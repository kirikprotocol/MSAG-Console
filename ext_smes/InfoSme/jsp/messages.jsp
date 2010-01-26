<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.*, ru.novosoft.smsc.infosme.backend.Message,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 ru.novosoft.smsc.infosme.beans.Messages,
                 java.io.*"%>
<%@ page import="ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataItem"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper"%>
<jsp:useBean id="infoSmeMessagesBean" scope="session" class="ru.novosoft.smsc.infosme.beans.Messages" />
<% infoSmeMessagesBean.clean(); %>
<jsp:setProperty name="infoSmeMessagesBean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

  Messages bean = infoSmeMessagesBean;
  int rowN = 0;
  int beanResult = bean.process(request);
  if (beanResult == Messages.RESULT_EXPORT_ALL) {
    bean.exportAll(response, out, true);
    return;
  } else if (beanResult == Messages.RESULT_EXPORT_ALL_NO_TEXTS) {
    bean.exportAll(response, out, false);
    return;
  } else if (beanResult == Messages.RESULT_UPDATE_ALL) {
    session.setAttribute("Export mess done",null);    
    request.getRequestDispatcher("updateMessages.jsp").forward(request, response);
    return;
  } if (beanResult == Messages.RESULT_UPDATE || beanResult == Messages.RESULT_CANCEL_UPDATE) {
    beanResult = Messages.RESULT_OK;
  } else {
    session.setAttribute("Export mess done",null);
  }
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="inc/header.jsp"%>

<div class=content>
<%--<input type=hidden name=initialized id=initialized value=<%=bean.isInitialized()%>>--%>
<%Collection allTasks = bean.getAllTasks(request);
  if (allTasks.size() > 0) {%>
<table class=properties_list>
<col width="10%">
<col width="40%">
<col width="10%">
<col width="40%">
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left"><%= getLocString("infosme.label.task")%>:</th>
  <td><%=StringEncoderDecoder.encode(bean.getTaskName())%></td>
  <th style="text-align:left"><%= getLocString("infosme.label.msg_state")%></th>
  <td><select name=status>
        <option value="<%=Message.State.UNDEFINED.getId()%>" <%= (bean.isStatus(Message.State.UNDEFINED.getId())) ? "selected":""%>>ALL</option>
        <option value="<%=Message.State.NEW.getId()%>"       <%= (bean.isStatus(Message.State.NEW.getId())) ? "selected":""%>      >NEW</option>
        <option value="<%=Message.State.WAIT.getId()%>"      <%= (bean.isStatus(Message.State.WAIT.getId())) ? "selected":""%>     >WAIT</option>
        <option value="<%=Message.State.ENROUTE.getId()%>"   <%= (bean.isStatus(Message.State.ENROUTE.getId())) ? "selected":""%>     >ENROUTE</option>
        <option value="<%=Message.State.DELIVERED.getId()%>" <%= (bean.isStatus(Message.State.DELIVERED.getId())) ? "selected":""%>>DELIVERED</option>
        <option value="<%=Message.State.EXPIRED.getId()%>"   <%= (bean.isStatus(Message.State.EXPIRED.getId())) ? "selected":""%>  >EXPIRED</option>
        <option value="<%=Message.State.FAILED.getId()%>"    <%= (bean.isStatus(Message.State.FAILED.getId())) ? "selected":""%>   >FAILED</option>
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

</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "infosme.hint.query_messages");
page_menu_button(session, out, "mbClear",  "common.buttons.clear",  "common.buttons.clearFilter");
page_menu_space(out);
page_menu_end(out);
}
else {
%>
<div style="color:blue"><%=getLocString("infosme.warn.no_task_for_msg")%></div>
<%
}
if (bean.isInitialized()) {
%>
<div class=content>
  <%{final PagedStaticTableHelper tableHelper = bean.getTableHelper();%>
  <%@ include file="/WEB-INF/inc/paged_static_table.jsp"%>
  <%}%>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbDelete",    "infosme.button.delete_msg", "infosme.hint.delete_msg");
page_menu_button(session, out, "mbResend",    "infosme.button.resend_msg", "infosme.hint.resend_msg");
page_menu_space(out);
page_menu_button(session, out, "mbUpdateAll", "infosme.button.update_all", "infosme.hint.update_all");
page_menu_button(session, out, "mbDeleteAll", "infosme.button.delete_all", "infosme.hint.delete_all");
page_menu_button(session, out, "mbResendAll", "infosme.button.resend_all", "infosme.hint.resend_all");
page_menu_button(session, out, "mbExportAll", "infosme.button.export_all", "infosme.hint.export_all");
page_menu_button(session, out, "mbExportAllNoTexts", "infosme.button.export_all_no_texts", "infosme.hint.export_all");
page_menu_end(out);
}%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>