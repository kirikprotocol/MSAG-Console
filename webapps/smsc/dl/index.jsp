<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.dl.*,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 java.util.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.admin.dl.DistributionList,
                 ru.novosoft.smsc.jsp.PageBean"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.dl.DistributionListAdminFormBean" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Distribution lists";
  MENU0_SELECTION = "MENU0_DL";

  int beanResult = bean.RESULT_OK;
  switch(beanResult = bean.process(request))  {
    case PageBean.RESULT_OK:
    case PageBean.RESULT_ERROR:
      break;
    case DistributionListAdminFormBean.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case DistributionListAdminFormBean.RESULT_EDIT:
      response.sendRedirect("dlEdit.jsp?name=" + URLEncoder.encode(bean.getEditDl()));
      return;
    case DistributionListAdminFormBean.RESULT_ADD:
      response.sendRedirect("dlAdd.jsp");
      return;
    case DistributionListAdminFormBean.RESULT_FILTER:
      response.sendRedirect("dlFilter.jsp");
      return;
    default:
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",  "Add distribution list",  "Create new distribution list");
page_menu_button(session, out, "mbDelete", "Delete distribution list(s)", "Delete selected distribution list(s)");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
<input type=hidden name=editDl>
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
function edit(dlName)
{
	document.all.jbutton.name = "mbEdit";
	opForm.editDl.value = dlName;
	opForm.submit();
	return false;
}
</script>

<table class=list cellspacing=0>
<col width="1%">
<col width="79%" align=left>
<col width="20%" align=left>
<thead>
<tr>
	<th>&nbsp;</th>
	<th><a href="#" <%=bean.getSort().endsWith("name")  ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by name"  onclick='return setSort("name") '>name</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("owner") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by owner" onclick='return setSort("owner")'>owner</a></th>
</tr>
</thead>
<tbody><%
int row = 0;
for (Iterator i = bean.getDls().iterator(); i.hasNext(); )
{
  DistributionList dl = (DistributionList) i.next();
	String name = dl.getName();
	String encName = StringEncoderDecoder.encode(name);
  String encOwner = StringEncoderDecoder.encode(dl.getOwner() == null || dl.getOwner().trim().length() == 0 ? "SYSTEM" : dl.getOwner());
	%>
	<tr class=row<%=(row++)&1%>>
		<td class=check><input class=check type=checkbox name=checkedDls value="<%=encName%>" <%=bean.isDlChecked(name) ? "checked" : ""%>></td>
		<td class=name><a href="#" title="Edit distribution list" onClick='return edit("<%=encName%>")'><%=encName%></a></td>
		<td><%=encOwner%></td>
	</tr><%
}
%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar.jsp"%>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",  "Add distribution list",  "Create new distribution list");
page_menu_button(session, out, "mbDelete", "Delete distribution list(s)", "Delete selected distribution list(s)");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>