<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.*,
                 ru.novosoft.smsc.jsp.smsc.acl.Edit,
                 ru.novosoft.smsc.admin.acl.AclInfo"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.acl.Edit"/>
<jsp:setProperty name="bean" property="*"/>
<%
  final boolean add = bean.isAdd();
  TITLE = add ? " Create Access Control List" : "Edit Access Control List";
  switch(bean.process(request))
  {
    case Edit.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case Edit.RESULT_OK:
      break;
    case Edit.RESULT_ERROR:
      break;
    default:
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ACL";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%><%
  page_menu_begin(out);
  page_menu_button(out, "mbSave",  add ? "Add" : "Save",  add ? "Add new acl" : "Save changes");
  page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>
<input type="hidden" name="initialized" value="true">
<input type="hidden" name="add" value="<%=add%>">
<script>
var global_counter = 0;
function addAclAddress()
{
  var valueElem = opForm.all.new_address;
  var addr = valueElem.value;
  if (addr != null && addr.length > 0) {
    var tbl = opForm.all.aclAddresses;
    var newRow = tbl.insertRow(tbl.rows.length);
		newRow.className = "row" + ((tbl.rows.length+1) & 1);
    newRow.id = "addrRow_" + (global_counter++);
    newCell = document.createElement("td");
    newCell.innerHTML = '<input name=addresses value="'+addr+'">';
    newRow.appendChild(newCell);
    newCell = document.createElement("td");
    newCell.innerHTML = '<img src="/images/but_del.gif" onClick="removeRow(opForm.all.aclAddresses, \'' + newRow.id + '\')" style="cursor: hand;">';
    newRow.appendChild(newCell);
    valueElem.value = "";
    valueElem.focus();
    return false;
  } else
    return false;
}
</script>
<div class=content>
  <div class="page_subtitle"><%=add ? "New Access Control List" : "Access Control List"%></div>
  <table class=properties_list cellspacing=0 cellspadding=0>
  <col width="15%">
  <col width="85%">
  <tr class=row0>
  	<td>id</td>
  	<td><input class=txt name=id value="<%=bean.getId()%>" validation="positive" onkeyup="resetValidation(this)" <%=add ? "" : "readonly"%>></td>
  </tr>
  <tr class=row1>
  	<td>name</td>
  	<td><input class=txt name="name" value="<%=bean.getName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
  </tr>
  <tr class=row0>
  	<td>description</td>
  	<td><input class=txt name="description" value="<%=bean.getDescription()%>"></td>
  </tr>
  </table>
  <table class=list cellspacing=0 cellspadding=0 id=aclAddresses>
  <col width="15%">
  <col width="85%">
  <tr>
  	<th colspan="2">adresses</th>
  </tr>
  <tr class=row0>
  	<td><input class=txt name="addresses" id="new_address"></td>
  	<td><%addButton(out, "add address", "add address", "add address", "return addAclAddress()");%></td>
  </tr>
  </table>
</div><%
  page_menu_begin(out);
  page_menu_button(out, "mbSave",  add ? "Add" : "Save",  add ? "Add new acl" : "Save changes");
  page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
