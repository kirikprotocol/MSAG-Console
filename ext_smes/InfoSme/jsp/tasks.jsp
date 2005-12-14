<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.Tasks,
                 java.net.URLEncoder, java.util.Iterator,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Tasks" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

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
<div class=content>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=edit>
<input type=hidden name=sort>
<%if (bean.isSmeRunning()) {%>
  <OBJECT id="tdcTasksStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
    <PARAM NAME="DataURL" VALUE="/smsc/smsc/esme_InfoSme/taskStatuses.jsp">
    <PARAM NAME="UseHeader" VALUE="True">
    <PARAM NAME="TextQualifier" VALUE='"'>
  </OBJECT>
  <script type="text/javascript">
    function refreshTaskStatuses()
    {
      document.getElementById('tdcTasksStatuses').DataURL = document.getElementById('tdcTasksStatuses').DataURL;
      document.getElementById('tdcTasksStatuses').reset();
      window.setTimeout(refreshTaskStatuses, 1000);
    }
    refreshTaskStatuses();
  </script>
<%}%>
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
<%
  QueryResultSet tasks = bean.getTasks();
  if (tasks.size() == 0)
  {%>No tasks found<%}
  else
  {%>
    <table class=list cellspacing=0>
    <col width="1%">
    <col width="90%"            > <!--name-->
    <col width="1%"             > <!--provider-->
    <col width="1%" align=center> <!--enabled-->
    <col width="1%" align=center> <!--generating-->
    <col width="1%" align=center> <!--processing-->
    <col width="1%" align=center> <!--priority-->
    <col width="1%" align=center> <!--retryOnFail-->
    <col width="1%" align=center> <!--replaceMessage-->
    <col width="1%" align=center> <!--trackIntegrity-->
    <col width="1%"             > <!--svcType-->
    <tr>
      <th>&nbsp;</th>
      <th><a href="#" <%=bean.getSort().endsWith("name")           ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Task name"            onclick='return setSort("name")'           >Task</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("provider")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Data provider name"   onclick='return setSort("provider")'       >Data provider</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("enabled")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Enabled flag"         onclick='return setSort("enabled")'        >Enabled</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("generating")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Generating flag"      onclick='return setSort("generating")'     >Generating</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("processing")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Processing flag"      onclick='return setSort("processing")'     >Processing</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("priority")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Priority"             onclick='return setSort("priority")'       >Priority</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("retryOnFail")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Retry on fail flag"   onclick='return setSort("retryOnFail")'    >Retry</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("replaceMessage") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Replace message flag" onclick='return setSort("replaceMessage")' >Replace</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("trackIntegrity") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Track integrity flag" onclick='return setSort("trackIntegrity")' >Integrity</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("svcType")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Service type"         onclick='return setSort("svcType")'        >SVC</a></th>
    </tr>
    <%
    int rowN = 0;
    for (Iterator i = tasks.iterator(); i.hasNext();) {
      TaskDataItem task = (TaskDataItem) i.next();

      String id = task.getId();
      String name = task.getName();
      String provider = task.getProvider();
      int priority = task.getPriority();
      boolean enabled = task.isEnabled();
      boolean generating = task.isGenerating();
      boolean processing = task.isProcessing();
      boolean retryOnFail = task.isRetryOnFail();
      boolean replaceMessage = task.isReplaceMessage();
      boolean trackIntegrity = task.isTrackIntegrity();
      String svcType = task.getSvcType();

      String idEnc = StringEncoderDecoder.encode(id);
      String idHex = StringEncoderDecoder.encodeHEX(id);
      String nameEnc = StringEncoderDecoder.encode(name);
      String providerEnc = StringEncoderDecoder.encode(provider);
      String svcTypeEnc = StringEncoderDecoder.encode(svcType);

      %><tr class=row<%=rowN++&1%>>
        <td><%if (bean.isSmeRunning()) {%><input class=check type=checkbox name=checked value="<%=idEnc%>" <%=bean.isTaskChecked(id) ? "checked" : ""%> onclick="checkCheckboxesForMbDeleteButton();"><%}else{%>&nbsp;<%}%></td>
        <td><a href="javascript:editSomething('<%=idEnc%>')" title="Edit task"><%=nameEnc%></a></td>
        <td nowrap><%=providerEnc%></td>
        <td><%if (enabled       ){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
        <td><span datasrc=#tdcTasksStatuses DATAFORMATAS=html datafld="gen<%=idHex%>"><%if (generating    ){%><img src="/images/ic_running.gif"><%}else{%><img src="/images/ic_stopped.gif"><%}%></span></td>
        <td><span datasrc=#tdcTasksStatuses DATAFORMATAS=html datafld="prc<%=idHex%>"><%if (processing    ){%><img src="/images/ic_running.gif"><%}else{%><img src="/images/ic_stopped.gif"><%}%></span></td>
        <td><%=priority%></td>
        <td><%if (retryOnFail   ){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
        <td><%if (replaceMessage){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
        <td><%if (trackIntegrity){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
        <td nowrap><%=svcTypeEnc == null || svcTypeEnc.trim().length() == 0 ? "&nbsp;" : svcTypeEnc%></td>
      </tr><%
    }
    %></table><%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%><%
  }%>
</div><%
page_menu_begin(out);
if (bean.isSmeRunning()) {
  page_menu_button(session, out, "mbAdd",    "common.buttons.add",    "infosme.hint.add_task");
  page_menu_button(session, out, "mbDelete", "common.buttons.delete", "infosme.hint.del_tasks", "return confirm('"+getLocString("infosme.confirm.del_tasks")+"');");
}
page_menu_space(out);
page_menu_end(out);
%>
<script type="text/javascript">
<%if (bean.isSmeRunning()) {%>checkCheckboxesForMbDeleteButton();<%}%>
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>