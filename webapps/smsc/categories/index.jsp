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
TITLE = getLocString("categories.title");
switch(bean.process(request))
{
	case IndexProvider.RESULT_DONE:
		response.sendRedirect("index.jsp"); // + "?startPosition=" + bean.getStartPositionInt());
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

   page_menu_button(session, out, "mbAdd",  "categories.add",  "categories.addHint");
   //page_menu_button(out, "mbDelete", "categories.delete", "categories.deleteHint");
   page_menu_space(out);
   page_menu_end(out);
 %><div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editName id=editName>
<input type=hidden name=editId id=editId>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>

<script>
function edit(id_to_edit,name_to_edit)
{
	document.getElementById('jbutton').name = "mbEdit";
	opForm.editName.value = name_to_edit;
  opForm.editId.value = id_to_edit;
  opForm.submit();
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
	document.getElementById('jbutton').name = "mbAddProvider";
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
  if (_body.style.display == 'none' || _body.style.display == '')
  {
    _head.className = 'collapsing_list_opened';
    _body.style.display = 'block';
  }
  else
  {
    _head.className = 'collapsing_list_closed';
    _body.style.display = 'none';
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
	<th width="14%" align=left><a href="javascript:setSort('id')" <%=bean.getSort().endsWith("id")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.idHint")%>"><%=getLocString("common.sortmodes.id")%></a></th>
	<th width="20%" align=left><a href="javascript:setSort('name')" <%=bean.getSort().endsWith("name")  ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.nameHint")%>"><%=getLocString("common.sortmodes.name")%></a></th>
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
  <td><a href="javascript:edit('<%=encCategoryId%>','<%=encName%>')" title="<%=getLocString("categories.editTitle")%>"><%=encName%></a></td>
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
   page_menu_button(session, out, "mbAdd",  "categories.add",  "categories.addHint");
//   page_menu_button(out, "mbDelete", "categories.delete", "categories.deleteHint");
   page_menu_space(out);
   page_menu_end(out);
 %>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
