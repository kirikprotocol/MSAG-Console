<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.subjects.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.*,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 java.net.URLEncoder"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.subjects.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("subjects.title");
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
		response.sendRedirect("subjectsFilter.jsp");
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("subjectsAdd.jsp");
		return;
	case Index.RESULT_EDIT:
		response.sendRedirect("subjectsEdit.jsp?name="+URLEncoder.encode(bean.getEditName()));
		return;
	default:

		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SUBJECTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<script>
function removeFilterMask(rowId)
{
  var t = opForm.all.filterMasksTable;
  var r = t.rows(rowId);
  if (r != null)
    t.deleteRow(r.rowIndex);
}

var rowCounter = 0;

function addFilterMask()
{
  var fme = opForm.all.newFilterMask;
  var fm = fme.value;
  if (fm == null || fm == "")
    return false;

  var t = opForm.all.filterMasksTable;
  var r = t.insertRow(t.rows.length -1);
  r.id = "row_FilterMask_" + rowCounter++;
  var c1 = r.insertCell();
  c1.innerHTML = "<input class=txt name=filter_masks value=\"" + fm + "\" validation=\"mask\" onkeyup=\"resetValidation(this)\">";
  var c2 = r.insertCell();
  c2.innerHTML = "<img src=\"/images/but_del.gif\" onclick=\"removeFilterMask('" + r.id + "')\" style=\"cursor:hand;\">";

  fme.value = "";
  fme.focus();
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
<div class=page_subtitle><%=getLocString("subjects.filterMasks")%></div>
<table id=filterMasksTable>
<col width="1%">
<col width="1%">
<col width="98%">
<%
  for (int i = 0; i < bean.getFilter_masks().length; i++) {
    final String filterMask = bean.getFilter_masks()[i];
    final String filterMaskHex = StringEncoderDecoder.encodeHEX(filterMask);
    %><tr id=filterMaskRow_<%=filterMaskHex%>>
      <td><input class=txt name=filter_masks value="<%=StringEncoderDecoder.encode(filterMask)%>" validation="mask" onkeyup="resetValidation(this)"></td>
      <td><img src="/images/but_del.gif" onclick="removeFilterMask('filterMaskRow_<%=filterMaskHex%>')" style="cursor:hand;"></td>
      <td>&nbsp;</td>
    </tr><%
  }
%>
<tr>
  <td><input class=txt name=filter_masks value="" id=newFilterMask validation="mask" onkeyup="resetValidation(this)"></td>
  <td><img src="/images/but_add.gif" onclick="addFilterMask()" style="cursor:hand;"></td>
  <td>&nbsp;</td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "subjects.queryHint");
page_menu_space(out);
page_menu_end(out);%>
<div class=content>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editName>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
<input type=hidden name=initialized value=true>

<script>
function edit(name_to_edit)
{
	document.all.jbutton.name = "mbEdit";
	opForm.editName.value = name_to_edit;
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
function navigate(direction)
{
	document.all.jbutton.name = direction;
	document.all.jbutton.value = direction;
	opForm.submit();
	return false;
}
function clickMasks(idSuffix)
{
  var h = opForm.all("masks_header_" + idSuffix);
  var b = opForm.all("masks_body_"   + idSuffix);
  if (h.className == "collapsing_list_closed") {
    h.className = "collapsing_list_opened";
    b.runtimeStyle.display = "block";
  } else {
    h.className = "collapsing_list_closed";
    b.runtimeStyle.display = "none";
  }
}
</script>
<table class=list cellspacing=1 width="100%">
<col width="1%">
<col width="20%" align=left>
<col width="20%" align=left>
<col width="59%" align=left>
<thead>
<tr>
	<th>&nbsp;</th>
	<th><a href="#" <%=bean.getSort().endsWith("Name") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.nameHint")%>" onclick='return setSort("Name")'><%=getLocString("common.sortmodes.name")%></a></th>
	<th><a href="#" <%=bean.getSort().endsWith("Default SME") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("subjects.sortDefaultSmeHint")%>" onclick='return setSort("Default SME")'><%=getLocString("subjects.sortDefaultSme")%></a></th>
	<th><a href="#" <%=bean.getSort().endsWith("Masks") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.masksHint")%>" onclick='return setSort("Masks")'><%=getLocString("common.sortmodes.masks")%></a></th>
</tr>
</thead>
<tbody>
<%{
final int MASKS_HEADER_SIZE = 1;
int row = 0;
for(Iterator i = bean.getSubjects().iterator(); i.hasNext(); row++)
{
  DataItem item = (DataItem) i.next();
  String name = (String)item.getValue("Name");
  String defSme = (String)item.getValue("Default SME");
  Vector masks = (Vector)item.getValue("Masks");
  String encName = StringEncoderDecoder.encode(name);
  String encDefSme = StringEncoderDecoder.encode(defSme);
  String hexName = StringEncoderDecoder.encodeHEX(name);
  String notes = (String) item.getValue("notes");
  if (notes == null)
    notes = "";

  String encNotes = StringEncoderDecoder.encode(notes);
  String rowId = "ROUTE_ID_" + StringEncoderDecoder.encodeHEX(name);
  String onClick = encNotes.length() > 0 ? " class=clickable onClick='clickClickable(\""+rowId+"_HEAD\", \"" + rowId + "_BODY\")'" : "";
%>
<tr class=row<%=row&1%>>
	<td class=check><input class=check type=checkbox name=checkedSubjects value="<%=encName%>" <%=bean.isSubjectChecked(name) ? "checked" : ""%>></td>
	<td <%=onClick%>><div id=<%=rowId%>_HEAD <%=encNotes.length() > 0 ? "class=collapsing_list_closed" : "class=collapsing_list_empty"%>><a href="#" title="<%=getLocString("subjects.editSubTitle")%>" onClick='return edit("<%=encName%>")'><%=encName%></a></div></td>
	<td <%=onClick%>><%=encDefSme%></td>
	<td><%
    if (masks.size() > 0)
    {
      if (masks.size() > MASKS_HEADER_SIZE) {
        %><div class=collapsing_list_closed id="masks_header_<%=hexName%>" onClick="clickMasks('<%=hexName%>')"><%
      }
      final int header_length = Math.min(MASKS_HEADER_SIZE, masks.size());
      for (int j = 0; j < header_length; j++) {
        String mask = (String) masks.elementAt(j);
        String encMask = StringEncoderDecoder.encode(mask);
        out.print(encMask);
        if (j < header_length-1)
          out.print(", ");
      }
      if (masks.size() > MASKS_HEADER_SIZE) {
        %>, ...</div><div id="masks_body_<%=hexName%>" class=collapsing_list_empty style="display: none;"><%
      }

      for (int j = MASKS_HEADER_SIZE; j < masks.size(); j++) {
        String mask = (String) masks.elementAt(j);
        String encMask = StringEncoderDecoder.encode(mask);
        out.print(encMask);
        if (j < masks.size()-1)
          out.print("<br>");
      }

      if (masks.size() > MASKS_HEADER_SIZE) {
        %></div><%
      }
    } else {
      out.print(getLocString("common.util.none"));
    }
  %></td>
</tr>
<tr class=row<%=row&1%> id=<%=rowId%>_BODY style="display:none">
  <td>&nbsp;</td>
  <td colspan=3><%=encNotes%></td>
</tr>
<%}}%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar.jsp"%>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",  "subjects.add",  "subjects.addHint");
page_menu_button(session, out, "mbDelete", "subjects.delete", "subjects.deleteHint");
if (bean.getAppContext().getStatuses().isRoutesChanged() || bean.getAppContext().getStatuses().isSubjectsChanged())
  if (!bean.getAppContext().getStatuses().isRoutesRestored())
    page_menu_button(session, out, "mbSave", "common.buttons.saveCurrent", "routes.saveCurrentHint");
if (bean.getAppContext().getStatuses().isRoutesSaved() && !bean.getAppContext().getStatuses().isRoutesRestored())
    page_menu_button(session, out, "mbRestore", "common.buttons.loadSaved", "routes.loadSavedHint");
if (bean.getAppContext().getStatuses().isRoutesChanged() || bean.getAppContext().getStatuses().isSubjectsChanged())
    page_menu_button(session, out, "mbLoad", "common.buttons.restoreApplied", "routes.restoreAppliedHint");
page_menu_space(out);
page_menu_end(out);%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
