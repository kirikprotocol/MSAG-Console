<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem,
				 ru.novosoft.smsc.admin.route.SourceList,
				 ru.novosoft.smsc.admin.route.DestinationList
                 ,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 java.util.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.text.DateFormat"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.routes.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%@ taglib uri="http://jakarta.apache.org/taglibs/input-1.0" prefix="input" %>

<%
TITLE = "Routes";
switch(bean.process(request))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		break;
	case Index.RESULT_ERROR:
		break;
	case Index.RESULT_FILTER:
		response.sendRedirect("routesFilter.jsp");
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("routesAdd.jsp");
		return;
	case Index.RESULT_EDIT:
		response.sendRedirect("routesEdit.jsp?routeId="+URLEncoder.encode(bean.getEditRouteId()));
		return;
	default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ROUTES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(out, "mbAdd",  "Add route",  "Add new route");
page_menu_button(out, "mbDelete", "Delete route(s)", "Delete selected route(s)");
if (bean.getAppContext().getStatuses().isRoutesChanged() || bean.getAppContext().getStatuses().isSubjectsChanged())
  if (!bean.getAppContext().getStatuses().isRoutesRestored())
    page_menu_button(out, "mbSave", "Save current", "Save current routing configuration");
if (bean.getAppContext().getStatuses().isRoutesSaved() && !bean.getAppContext().getStatuses().isRoutesRestored())
    page_menu_button(out, "mbRestore", "Load saved", "Load saved routing configuration");
if (bean.getAppContext().getStatuses().isRoutesChanged() || bean.getAppContext().getStatuses().isSubjectsChanged())
    page_menu_button(out, "mbLoad", "Restore applied", "Restore applied routing configuration");
page_menu_space(out);
page_menu_end(out);%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editRouteId>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>

<script>
function edit(name_to_edit)
{
	document.all.jbutton.name = "mbEdit";
	opForm.editRouteId.value = name_to_edit;
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

function clickFilterSelect()
{
	document.all.jbutton.name = "mbQuickFilter";
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

<table class=page_menu with="100%" >
<%--<tr>
<td><input:select name="filterSelect" default="1"
    attributes="<%= as %>" options="<%= o %>"  /></td></tr> --%>

<tr><td align="left" with="10%"> Filter By: &nbsp;  </td><td align="left" with="90%">  &nbsp;  </td>

<%
page_small_menu_begin(out);
page_menu_button(out, "mbQuickFilter",  "Apply",  "Apply filter","return clickFilterSelect()",true);
page_menu_button(out, "mbClear", "Clear", "Clear filter", "clickClear()");
//page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing", "clickCancel()");
page_menu_space(out);
page_small_menu_end(out);
%>
</tr>
</table>
<hr>
<%
java.util.HashMap as = new java.util.HashMap();
// as.put("multiple", null);
// as.put("onchange", "return setSort('Route ID')");
 as.put("size", "10");

  java.util.HashMap aq = new java.util.HashMap();
// aq.put("multiple", null);
 aq.put("onenter", "return setSort('Route ID')");

java.util.TreeMap o = new java.util.TreeMap();
o.put("Sources Subj", "1");
o.put("Sources Mask", "2");
o.put("Dest Subj", "3");
o.put("Dest Mask", "4");
o.put("SMEs", "5");
%>
<table  with="100%">
<%--<tr>
<td><input:select name="filterSelect" default="1"
    attributes="<%= as %>" options="<%= o %>"  /></td></tr> --%>

<tr>
<td align="left" with="10%">    Name: &nbsp;</td><td with="40%" align="left"><input:text name="queryName"  default="<%=bean.getQueryName()%>"/> </td>
<td align="left" with="10%">    Subj: &nbsp;</td><td with="40%" align="left"><input:text name="querySubj"  default="<%=bean.getQuerySubj()%>"/> </td>
</tr>
<tr>
<td align="left" with="10%">    SMEs: &nbsp;</td><td with="40%" align="left"><input:text name="querySMEs" default="<%=bean.getQuerySMEs()%>"/>  </td>
<td align="left" with="10%">    Mask: &nbsp;</td><td with="40%" align="left"><input:text name="queryMask" default="<%=bean.getQueryMask()%>"/>  </td>
</tr>
<tr><td colspan="2">  &nbsp;&nbsp;

<%--button(out, "but_filter.gif", "filterSelect", "Apply", "Filter", "return clickFilterSelect()"); --%>
  </td>
  </tr>
 </table>
<table class=list cellspacing=0 cellpadding=0 id=ROUTE_LIST_TABLE>
<col width="1%">
<col width="60%" align=left>
<col width="20%" align=center>
<col width="20%" align=center>
<col width="20%" align=center>
<col width="20%" align=center>
<col width="20%" align=center>
<thead>
<tr>
	<th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th><a href="#" <%=bean.getSort().endsWith("Route ID")                ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by name" onclick='return setSort("Route ID")'                        >name</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("active")                  ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by active status" onclick='return setSort("active")'                 >active</a></th>

	<th><a href="#" <%=bean.getSort().endsWith("isEnabling")              ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by active status" onclick='return setSort("isEnabling")'             >allow</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("isBilling")               ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by active status" onclick='return setSort("isBilling")'              >billing</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("isArchiving")             ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by active status" onclick='return setSort("isArchiving")'            >archiving</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("suppressDeliveryReports") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by active status" onclick='return setSort("suppressDeliveryReports")'>reports</a></th>
</tr>
</thead>
<tbody>
<%{
int row = 0;
for(Iterator i = bean.getRoutes().iterator(); i.hasNext(); row++)
{
  DataItem item = (DataItem) i.next();
  String routeId = (String) item.getValue("Route ID");
  SourceList sources = (SourceList) item.getValue("sources");
  DestinationList destinations = (DestinationList) item.getValue("destinations");
  boolean isActive                  = ((Boolean)item.getValue("active"                 )).booleanValue();
  boolean isEnabling                = ((Boolean)item.getValue("isEnabling"             )).booleanValue();
  boolean isBilling                 = ((Boolean)item.getValue("isBilling"              )).booleanValue();
  boolean isArchiving               = ((Boolean)item.getValue("isArchiving"            )).booleanValue();
  boolean isSuppressDeliveryReports = ((Boolean)item.getValue("suppressDeliveryReports")).booleanValue();
  String notes                      = (String) item.getValue("notes");
  if (notes == null)
    notes = "";

  String encRouteId = StringEncoderDecoder.encode(routeId);
  String encNotes = StringEncoderDecoder.encode(notes);
  String rowId = "ROUTE_ID_" + StringEncoderDecoder.encodeHEX(routeId);
  String onClick = encNotes.length() > 0 ? " class=clickable onClick='clickClickable(\""+rowId+"_HEAD\", \"" + rowId + "_BODY\")'" : "";
%>
<tr class=row<%=row&1%>>
	<td><input class=check type=checkbox name=checkedRouteIds value="<%=encRouteId%>" <%=bean.isRouteChecked(routeId) ? "checked" : ""%>></td>
	<td <%=onClick%>><div id=<%=rowId%>_HEAD <%=encNotes.length() > 0 ? "class=collapsing_list_closed" : "class=collapsing_list_empty"%>><a href="#" title="Edit route" onClick='return edit("<%=encRouteId%>")'><%=encRouteId%></a></div></td>
	<td <%=onClick%>><%if (isActive                 ){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
	<td <%=onClick%>><%if (isEnabling               ){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
	<td <%=onClick%>><%if (isBilling                ){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
	<td <%=onClick%>><%if (isArchiving              ){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
	<td <%=onClick%>><%if (isSuppressDeliveryReports){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
</tr>
<tr class=row<%=row&1%> id=<%=rowId%>_BODY style="display:none">
  <td>&nbsp;</td>
	<td colspan=6><%=encNotes%></td>
</tr>
<%}}%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar.jsp"%>
</div><%
  final Locale locale = request.getLocale();
  Calendar restoreCalendar = new GregorianCalendar(locale);
  DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.MEDIUM, locale);
  String restoreDate = null;
  final Date restoreFileDate = bean.getRestoreFileDate();
  if (restoreFileDate != null) {
    restoreCalendar.setTime(restoreFileDate);
    restoreDate = dateFormat.format(restoreFileDate);
  }
  String loadDate = null;
  final Date loadFileDate = bean.getLoadFileDate();
  if (loadFileDate != null) {
    restoreCalendar.setTime(loadFileDate);
    loadDate = dateFormat.format(loadFileDate);
  }

  page_menu_begin(out);
  page_menu_button(out, "mbAdd",  "Add route",  "Add new route");
  page_menu_button(out, "mbDelete", "Delete route(s)", "Delete selected route(s)");
  if (bean.getAppContext().getStatuses().isRoutesChanged() || bean.getAppContext().getStatuses().isSubjectsChanged())
    if (!bean.getAppContext().getStatuses().isRoutesRestored())
      page_menu_button(out, "mbSave", "Save current", "Save current routing configuration");
  if (bean.getAppContext().getStatuses().isRoutesSaved() && !bean.getAppContext().getStatuses().isRoutesRestored())
    page_menu_button(out, "mbRestore", "Load saved", "Load saved routing configuration",
                     restoreDate != null
                        ? "return confirm('Date of saved file is " + restoreDate + ". Are you sure to load this file?');"
                        : null
                     );
  if (bean.getAppContext().getStatuses().isRoutesChanged() || bean.getAppContext().getStatuses().isSubjectsChanged())
    page_menu_button(out, "mbLoad", "Restore applied", "Restore applied routing configuration",
                     loadDate != null
                        ? "return confirm('Date of restore file is " + loadDate + ". Are you sure to load this file?');"
                        : null
                     );
  page_menu_space(out);
  page_menu_end(out);%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
