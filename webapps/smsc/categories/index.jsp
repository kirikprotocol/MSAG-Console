<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.providers.IndexProvider,
                 ru.novosoft.smsc.jsp.util.tables.DataItem,
				 ru.novosoft.smsc.admin.route.SourceList,
				 ru.novosoft.smsc.admin.route.DestinationList,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.text.DateFormat,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 ru.novosoft.smsc.jsp.smsc.providers.IndexProvider"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.categories.IndexCategory"/>
<jsp:setProperty name="bean" property="*"/>
<%@ taglib uri="http://jakarta.apache.org/taglibs/input-1.0" prefix="input" %>

<%
TITLE = "Categories";
switch(bean.process(request))
{
	case IndexProvider.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case IndexProvider.RESULT_OK:

		break;
	case IndexProvider.RESULT_ERROR:

		break;
	case IndexProvider.RESULT_FILTER:
		response.sendRedirect("categoriesFilter.jsp");
		return;
	case IndexProvider.RESULT_ADD:
		response.sendRedirect("categoriesAdd.jsp");
		return;
	case IndexProvider.RESULT_EDIT:
		response.sendRedirect("categoriesEdit.jsp?name="+URLEncoder.encode(bean.getEditName())+"&&id="+URLEncoder.encode(bean.getEditId()));
		return;
default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_CATEGORIES";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
   page_menu_begin(out);

   page_menu_button(session, out, "mbAdd",  "Add category",  "Add new category");
  // page_menu_button(out, "mbDelete", "Delete category(ies)", "Delete selected category(ies)");
   page_menu_space(out);
   page_menu_end(out);
 %><div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editName>
<input type=hidden name=editId>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>

<script>
function edit(id_to_edit,name_to_edit)
{
	document.all.jbutton.name = "mbEdit";
	opForm.editName.value = name_to_edit;
  opForm.editId.value = id_to_edit;
  opForm.submit();
  window.event.cancelBubble = true;
	return false;
}
function setSort(sorting)
{
	if (sorting == "<%=bean.getSort()%>")
		opForm.sort.value = "-<%=bean.getSort()%>";
	else
		opForm.sort.value = sorting;
	opForm.submit();
	return false;
}

function clickAddSelect()
{
	document.all.jbutton.name = "mbAddProvider";
	opForm.submit();
	return false;
}
function setFilter(filtering)
{
	if (filtering == "<%=bean.getSort()%>")
		opForm.sort.value = "-<%=bean.getSort()%>";
	else
		opForm.sort.value = filtering;
	opForm.submit();
	return false;
}

function clickClickable(headId, bodyId)
{
  var _head = document.all(headId);
  var _body = document.all(bodyId);
  if (_body.runtimeStyle.display == 'none' || _body.runtimeStyle.display == '')
  {
    _head.className = 'collapsing_list_opened';
    _body.runtimeStyle.display = 'block';
  }
  else
  {
    _head.className = 'collapsing_list_closed';
    _body.runtimeStyle.display = 'none';
  }
}
</script>


<hr>
<table class=list cellspacing=0 cellpadding=0 id=CATEGORY_LIST_TABLE>
<col width="1%">
<col width="24%" align=left>
<col width="40%" align=left>
<col width="35%" align=left>
<thead>
<tr>
	<th width="1%" class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th width="14%" align=left><a href="#" <%=bean.getSort().endsWith("id")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by id" onclick='return setSort("id")'  >id</a></th>
	<th width="20%" align=left><a href="#" <%=bean.getSort().endsWith("name")  ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by name" onclick='return setSort("name")'  >name</a></th>
  <th width="65%" align=left>&nbsp </th>
  </tr>
</thead>
<tbody>
<%{
int row = 0;
   boolean have=true;
  QueryResultSet categories =(QueryResultSet) bean.getCategories();
  try {
      Iterator i =categories.iterator();
  } catch (Exception e) {   have=false;
    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
  }
if (have) {
  for(Iterator i =categories.iterator(); i.hasNext(); row++)
{
  DataItem item = (DataItem) i.next();
  String categoryId = (String) item.getValue("id");
  String encCategoryId = StringEncoderDecoder.encode(categoryId);
  String rowId = "ROUTE_ID_" + StringEncoderDecoder.encodeHEX(categoryId);
  String name = (String) item.getValue("name");
  String encName = StringEncoderDecoder.encode(name);

 %>
<tr class=row<%=row&1%>>
<td><input class=check type=checkbox name=checkedCategoryNames value="<%=encName%>" <%=bean.isCategoryChecked(name) ? "checked" : ""%>></td>
	<td><%=encCategoryId%>&nbsp;</td>
  <td><a href="#" title="Edit category" onClick='return edit("<%=encCategoryId%>","<%=encName%>")'><%=encName%></a></td>
</tr>
<tr class=row<%=row&1%> id=<%=rowId%>_BODY style="display:none">
  <td>&nbsp;</td>
	<td colspan=4></td>
</tr>
<%}
} //if (have) category
}%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar.jsp"%>
</div>
 <%
   page_menu_begin(out);
   page_menu_button(session, out, "mbAdd",  "Add category",  "Add new category");
  // page_menu_button(out, "mbDelete", "Delete category(ies)", "Delete selected category(ies)");
   page_menu_space(out);
   page_menu_end(out);
 %>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
