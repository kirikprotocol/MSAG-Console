<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.Tasks,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 java.util.Collection,
                 java.util.Iterator,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Tasks" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = Constants.INFO_SME_ID;
	TITLE="Informer SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.RESULT_OK;
	switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
	{
		case Tasks.RESULT_APPLY:
			response.sendRedirect("index.jsp");
			return;
		case Tasks.RESULT_OPTIONS:
			response.sendRedirect("options.jsp");
			return;
		case Tasks.RESULT_DRIVERS:
			response.sendRedirect("drivers.jsp");
			return;
		case Tasks.RESULT_PROVIDERS:
			response.sendRedirect("providers.jsp");
			return;
		case Tasks.RESULT_TASKS:
			response.sendRedirect("tasks.jsp");
			return;
		case Tasks.RESULT_SHEDULES:
			response.sendRedirect("shedules.jsp");
			return;
		case Tasks.RESULT_DONE:
			response.sendRedirect("index.jsp");
			return;
		case Tasks.RESULT_OK:
			STATUS.append("Ok");
			break;
    case Tasks.RESULT_EDIT:
      response.sendRedirect("taskEdit.jsp?sectionName=" + URLEncoder.encode(bean.getEdit(), "UTF-8"));
      return;
    case Tasks.RESULT_ADD:
      response.sendRedirect("taskEdit.jsp?create=true");
      return;
		case Tasks.RESULT_ERROR:
			STATUS.append("<span class=CF00>Error</span>");
			break;
		default:
			STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
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
  QueryResultSet tasks = bean.getTasks();
  if (tasks.size() == 0)
  {%>No tasks found<%}
  else
  {%>
    <table class=list cellspacing=0>
    <col width="1%">
    <col width="93%"            > <!--name-->
    <col width="1%"             > <!--provider-->
    <col width="1%" align=center> <!--enabled-->
    <col width="1%"             > <!--priority-->
    <col width="1%" align=center> <!--retryOnFail-->
    <col width="1%" align=center> <!--replaceMessage-->
    <col width="1%"             > <!--svcType-->
    <tr>
      <th>&nbsp;</th>
      <th><a href="#" <%=bean.getSort().endsWith("name")           ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by task name"           onclick='return setSort("name")'          >Task name</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("provider")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by data provider name"  onclick='return setSort("provider")'      >Data provider</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("enabled")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Enabled flag"        onclick='return setSort("enabled")'       >Enabled</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("priority")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by priority"            onclick='return setSort("priority")'      >Priority</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("retryOnFail")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by RetryOnFail flag"    onclick='return setSort("retryOnFail")'   >Retry on fail</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("replaceMessage") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by ReplaceMessage flag" onclick='return setSort("replaceMessage")'>Replace message</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("svcType")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by service type"        onclick='return setSort("svcType")'       >Service type</a></th>
    </tr>
    <%
    int rowN = 0;
    for (Iterator i = tasks.iterator(); i.hasNext();) {
      TaskDataItem task = (TaskDataItem) i.next();

      String name = task.getName();
      String provider = task.getProvider();
      int priority = task.getPriority();
      boolean enabled = task.isEnabled();
      boolean retryOnFail = task.isRetryOnFail();
      boolean replaceMessage = task.isReplaceMessage();
      String svcType = task.getSvcType();

      String nameEnc = StringEncoderDecoder.encode(name);
      String providerEnc = StringEncoderDecoder.encode(provider);
      String svcTypeEnc = StringEncoderDecoder.encode(svcType);

      %><tr class=row<%=rowN++&1%>>
        <td><input class=check type=checkbox name=checked value="<%=nameEnc%>" <%=bean.isTaskChecked(name) ? "checked" : ""%> onclick="checkCheckboxesForMbDeleteButton();"></td>
        <td><a href="#" title="Edit task" onClick='return editSomething("<%=nameEnc%>");'><%=nameEnc%></a></td>
        <td nowrap><%=providerEnc%></td>
        <td><%if (enabled       ){%><img src="<%=CPATH%>/img/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
        <td><%=priority%></td>
        <td><%if (retryOnFail   ){%><img src="<%=CPATH%>/img/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
        <td><%if (replaceMessage){%><img src="<%=CPATH%>/img/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
        <td nowrap><%=svcTypeEnc%></td>
      </tr><%
    }
    %></table><%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%><%
  }%>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbAdd",    "Add",    "Add new task");
page_menu_button(out, "mbDelete", "Delete", "Delete checked task(s)", "return confirm('Are you sure to delete all checked task(s)?');");
page_menu_space(out);
page_menu_end(out);
%>
<script>
checkCheckboxesForMbDeleteButton();
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>