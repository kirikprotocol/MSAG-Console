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
      String to = "taskEdit.jsp?id=" + URLEncoder.encode(bean.getEdit(), "UTF-8");
      if(bean.getState() == Tasks.ARCHIVE) {
        to+="&archiveDate="+bean.getViewArchiveStartDate();
      }
      response.sendRedirect(to);
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
    <td style="text-align:left"><%=getLocString("infosme.label.task")%>:</td>
    <td><input class="txt" type="text" name="name" value="<%=bean.getName()%>"></td>
    <td style="text-align:left"><%=getLocString("infosme.label.status")%></td>
    <td>
      <% int status = bean.getStatus(); %>
      <select name="status">
        <option value="0" <%=status == 0 ? "SELECTED": ""%>><%=getLocString("infosme.label.task.status.all")%></option>
        <option value="1" <%=status == 1 ? "SELECTED": ""%>><%=getLocString("infosme.label.task.status.active")%></option>
        <option value="2" <%=status == 2 ? "SELECTED": ""%>><%=getLocString("infosme.label.task.status.finished")%></option>
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
    <td style="text-align:left"><%=getLocString("infosme.label.view")%>:</td>
    <td>
      <% int view = bean.getView(); %>
      <select name="view" id="view" <%=bean.getState() == Tasks.ARCHIVE ? "disabled" : ""%>>
        <option value="<%=Tasks.VIEW_LIST%>" <%=view == Tasks.VIEW_LIST || bean.getState() == Tasks.ARCHIVE ? "SELECTED": ""%>><%=getLocString("infosme.label.task.view.list")%></option>
        <option value="<%=Tasks.VIEW_WEEKLY%>" <%=view == Tasks.VIEW_WEEKLY && bean.getState() != Tasks.ARCHIVE ? "SELECTED": ""%>><%=getLocString("infosme.label.task.view.weekly")%></option>
      </select>
    </td>
    <td style="text-align:left"><%=getLocString("infosme.label.state")%>:</td>
    <td>
      <% int state = bean.getState(); %>
      <select name="state" onchange="document.getElementById('view').disabled = (this.value == '<%=Tasks.ARCHIVE%>');document.getElementById('view').value='<%=Tasks.VIEW_LIST%>'">
        <option value="<%=Tasks.ACTIVE%>" <%=state == Tasks.ACTIVE ? "SELECTED": ""%>><%=getLocString("infosme.label.task.state.active")%></option>
        <option value="<%=Tasks.ARCHIVE%>" <%=state == Tasks.ARCHIVE ? "SELECTED": ""%>><%=getLocString("infosme.label.task.state.archivated")%></option>
      </select>
    </td>
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

<%   if(bean.getState() != Tasks.ACTIVE && request.getParameter("processing_cancel") == null &&
    (bean.getArchiveState() == Tasks.ARCHIVE_LOADING_FINISHED || bean.getArchiveState() == Tasks.ARCHIVE_LOADING_PROCESSING)) {   %>
  <script type="text/javascript">
    var statusDataSource = new StringTableDataSource({url: '/smsc/smsc/esme_InfoSme/tasks_progress.jsp?processing=', async: false});
  </script>

  Progress: <span id="tdcProgress" style='color:blue;'><%=bean.getProgress()%>%</span> (<a id="cancelButton" style="display:<%=bean.getProgress() == 100 ? "none" : ""%>" href="<%=request.getRequestURL()%>?processing_cancel="><%=getLocString("common.buttons.cancel")%></a><a id="nextButton" href="<%=request.getRequestURL()%>?viewResults=" style="display:<%=bean.getProgress() != 100 ? "none" : ""%>"><%=getLocString("common.buttons.next")%></a>)

  <script language="javascript" type="text/javascript">
    statusDataSource.hasObservers();
    statusDataSource.addObserver(new ElementObserver({elementId: 'tdcProgress', field: 'progress'}));

    function refreshProgressStatus() {
      statusDataSource.update();
      var text = document.getElementById("tdcProgress").innerText;
      if (text.trim() == "100%") {
        document.getElementById("nextButton").style.display="";
        document.getElementById("cancelButton").style.display="none";
      }
      else {
        document.getElementById("nextButton").style.display="none";
        document.getElementById("cancelButton").style.display="";
      }
      window.setTimeout(refreshProgressStatus, 2000);
    }

    window.setTimeout(refreshProgressStatus, 2000);
  </script>


<% }else if(request.getParameter("processing_cancel") == null) {
    final PagedStaticTableHelper tableHelper = bean.getTableHelper();
%>
<%@ include file="/WEB-INF/inc/paged_static_table.jsp"%>
<%}%>
</div>




<%
  if(bean.getState() != Tasks.ARCHIVE) {
    page_menu_begin(out);
    if (bean.isSmeRunning()) {
      if (admin)
        page_menu_button(session, out, "mbAdd",    "common.buttons.add",    "infosme.hint.add_task");
      if (view == Tasks.VIEW_LIST) {
        page_menu_confirm_button(session, out, "mbDelete", "common.buttons.delete", "infosme.hint.del_tasks", getLocString("infosme.confirm.del_tasks"));
        page_menu_confirm_button(session, out, "mbEnable",  "common.buttons.enable",  "infosme.hint.enable", getLocString("infosme.confirm.enable.tasks"));
        page_menu_confirm_button(session, out, "mbDisable", "common.buttons.disable", "infosme.hint.disable", getLocString("infosme.confirm.disable.tasks"));
        page_menu_confirm_button(session, out, "mbArchivate", "common.buttons.archivate", "infosme.hint.archivate", getLocString("infosme.confirm.archivate.tasks"));
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
    }
%>
<script type="text/javascript">
<%--<%if (bean.isSmeRunning()) {%>checkCheckboxesForMbDeleteButton();<%}%>--%>
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>