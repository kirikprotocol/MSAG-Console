<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.Tasks,
                 java.net.URLEncoder, java.util.Iterator,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper" %>
<jsp:useBean id="bean" scope="session" class="ru.novosoft.smsc.infosme.beans.Tasks" />
<jsp:setProperty name="bean" property="*"/>
<%
  boolean admin = InfoSmeBean.isUserAdmin(request);

	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";

  int rowN = 0;
	int beanResult = bean.process(request);
	switch(beanResult)                                                                                    
	{
    case Tasks.RESULT_EDIT:
      response.sendRedirect("taskEdit.jsp?id=" + URLEncoder.encode(bean.getEdit(), "UTF-8"));
      return;
    case Tasks.RESULT_ADD:
      response.sendRedirect("taskEdit.jsp?create=true");
      return;
		default:
      {
        %><%@ include file="inc/menu_switch.jsp"%><%
      }
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<div class=content>

<table class=properties_list>
  <col width="10%">
  <col width="40%">
  <col width="10%">
  <col width="40%">
  <tr class=row<%=rowN++&1%>>
    <td style="text-align:left">Task:</td>
    <td><input class="txt" type="text" name="name" value="<%=bean.getName()%>"></td>
    <td style="text-align:left">Status:</td>
    <td>
      <% int status = bean.getStatus(); %>
      <select name="status">
        <option value="0" <%=status == 0 ? "SELECTED": ""%>>ALL</option>
        <option value="1" <%=status == 1 ? "SELECTED": ""%>>ACTIVE</option>
        <option value="2" <%=status == 2 ? "SELECTED": ""%>>FINISHED</option>
      </select>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>>
    <td style="text-align:left"><%= getLocString("infosme.label.from_date")%></td>
    <td><input class=calendarField id=fromDate name=fromDate value="<%=StringEncoderDecoder.encode(bean.getFromDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
    <td style="text-align:left"><%= getLocString("infosme.label.till_date")%></td>
    <td><input class=calendarField id=tillDate name=tillDate value="<%=StringEncoderDecoder.encode(bean.getTillDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
  </tr>
  <tr class=row<%=rowN++&1%>>
    <td style="text-align:left">View:</td>
    <td>
      <% int view = bean.getView(); %>
      <select name="view">
        <option value="<%=Tasks.VIEW_LIST%>" <%=view == Tasks.VIEW_LIST ? "SELECTED": ""%>>LIST</option>
        <option value="<%=Tasks.VIEW_WEEKLY%>" <%=view == Tasks.VIEW_WEEKLY ? "SELECTED": ""%>>WEEKLY</option>
      </select>
    </td>
    <td colspan="2">&nbsp;</td>
  </tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "Query Tasks");
page_menu_button(session, out, "mbClear",  "common.buttons.clear",  "Clear Filter");
page_menu_space(out);
page_menu_end(out);
%>
<div class="content">
<%{final PagedStaticTableHelper tableHelper = bean.getTableHelper();
  tableHelper.fillTable(); %>
<%@ include file="/WEB-INF/inc/paged_static_table.jsp"%>
<%}%>
</div><%
page_menu_begin(out);
if (bean.isSmeRunning()) {
  if (admin)
    page_menu_button(session, out, "mbAdd",    "common.buttons.add",    "infosme.hint.add_task");
  if (view == Tasks.VIEW_LIST) {
    page_menu_confirm_button(session, out, "mbDelete", "common.buttons.delete", "infosme.hint.del_tasks", getLocString("infosme.confirm.del_tasks"));
    page_menu_confirm_button(session, out, "mbEnable",  "common.buttons.enable",  "infosme.hint.enable", getLocString("infosme.confirm.enable.tasks"));
    page_menu_confirm_button(session, out, "mbDisable", "common.buttons.disable", "infosme.hint.disable", getLocString("infosme.confirm.disable.tasks"));
  }
}

page_menu_space(out);
if (!admin) {
  page_menu_button(session, out, "mbApply",  "common.buttons.apply", "infosme.hint.apply_changes");
  page_menu_button(session, out, "mbReset",  "common.buttons.reset", "infosme.hint.reset_changes");
}
page_menu_confirm_button(session, out, "mbEnableAll",  "common.buttons.enable.all",  "infosme.hint.enable.all", getLocString("infosme.confirm.enable.tasks"));
page_menu_confirm_button(session, out, "mbDisableAll", "common.buttons.disable.all", "infosme.hint.disable.all", getLocString("infosme.confirm.disable.tasks"));
page_menu_end(out);
%>
<script type="text/javascript">
<%--<%if (bean.isSmeRunning()) {%>checkCheckboxesForMbDeleteButton();<%}%>--%>
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>