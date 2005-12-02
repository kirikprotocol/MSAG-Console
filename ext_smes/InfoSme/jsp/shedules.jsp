<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.infosme.beans.Shedules,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 java.util.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.infosme.backend.tables.schedules.ScheduleDataItem"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Shedules" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.process(request);
	switch(beanResult)
	{
    case Shedules.RESULT_EDIT:
      response.sendRedirect("scheduleEdit.jsp?name=" + URLEncoder.encode(bean.getEdit(), "UTF-8"));
      return;
    case Shedules.RESULT_ADD:
      response.sendRedirect("scheduleEdit.jsp?create=true");
      return;
		default:
      {
        %><%@ include file="inc/menu_switch.jsp"%><%
      }
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=edit>
<input type=hidden name=sort>
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
<%
  QueryResultSet schedules = bean.getSchedules();
  if (schedules.size() == 0)
  {%><%= getLocString("infosme.label.no_schedules")%><%}
  else
  {%>
    <table class=list cellspacing=0>
    <col width="1%">
    <col width="93%"            > <!--name-->
    <col width="1%"             >
    <col width="1%" align=center>
    <tr>
      <th>&nbsp;</th>
      <th><a href="#" <%=bean.getSort().endsWith("name")          ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by schedule name"  onclick='return setSort("name")'         >Schedule name</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("execute")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by schedule type"  onclick='return setSort("execute")'      >Type</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("startDateTime") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by start date"     onclick='return setSort("startDateTime")'>Start</a></th>
    </tr>
    <%
    int rowN = 0;
    for (Iterator i = schedules.iterator(); i.hasNext();) {
      ScheduleDataItem task = (ScheduleDataItem) i.next();

      String name = task.getName();
      String execute = task.getExecute();
      String startDateTime = task.getStartDateTime();

      String nameEnc = StringEncoderDecoder.encode(name);
      String executeEnc = StringEncoderDecoder.encode(execute);
      String startDateTimeEnc = StringEncoderDecoder.encode(startDateTime);

      %><tr class=row<%=rowN++&1%>>
        <td><input class=check type=checkbox name=checked value="<%=nameEnc%>" <%=bean.isScheduleChecked(name) ? "checked" : ""%> onclick="checkCheckboxesForMbDeleteButton();"></td>
        <td><a href="#" title="<%= getLocString("infosme.label.edit_task")%>" onClick='return editSomething("<%=nameEnc%>");'><%=nameEnc%></a></td>
        <td><%=executeEnc%></td>
        <td nowrap><%=startDateTimeEnc%></td>
      </tr><%
    }
    %></table><%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%><%
  }%>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",    "common.buttons.add",    "infosme.hint.add_schedule");
page_menu_button(session, out, "mbDelete", "common.buttons.delete", "infosme.hint.del_scheduls", "return confirm('"+getLocString("infosme.confirm.del_schedules")+"');");
page_menu_space(out);
page_menu_end(out);
%>
<script>
checkCheckboxesForMbDeleteButton();
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>