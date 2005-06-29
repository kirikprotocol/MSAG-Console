<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.*,
                 ru.novosoft.smsc.jsp.smsc.acl.Edit,
                 ru.novosoft.smsc.admin.acl.AclInfo"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.acl.Edit"/>
<jsp:setProperty name="bean" property="*"/>
<%
  final boolean add = bean.isAdd();
  TITLE = add ? getLocString("acl.addTitle") : getLocString("acl.editTitle");
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
<<<<<<< edit.jsp
  page_menu_button(session, out, "mbSave",  add ? "Add" : "Save",  add ? "Add new acl" : "Save changes");
  page_menu_button(session, out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
=======
  page_menu_button(session, out, "mbSave",  add ? "common.buttons.add" : "common.buttons.save",  add ? "acl.addNew" : "common.buttons.saveChanges");
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancelHint", "clickCancel()");
>>>>>>> 1.5
  page_menu_space(out);
  page_menu_end(out);
%>
<input type="hidden" name="initialized" value="true">
<input type="hidden" name="add" value="<%=add%>">
<input type="hidden" name="id" value="<%=bean.getId()%>">   
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
  <div class="page_subtitle"><%=add ? getLocString("acl.addSubTitle") : getLocString("acl.subTitle")%></div>
  <table class=properties_list cellspacing=0 cellspadding=0>
  <col width="15%">
  <col width="85%">
  <tr class=row1>
  	<td><%=getLocString("acl.name")%></td>
  	<td><input class=txt name="name" value="<%=bean.getName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
  </tr>
  <tr class=row0>
  	<td><%=getLocString("acl.description")%></td>
  	<td><input class=txt name="description" value="<%=bean.getDescription()%>"></td>
  </tr>
  <tr class=row1>
    <td><%=getLocString("acl.cacheType")%></td>
    <td><div class=select><select class=txt name="cache_type">
      <option value='0' <%=bean.getCache_type() == '0' ? "selected" : ""%>><%=getLocString("acl.cacheType.unknown")%></option>
      <option value='1' <%=bean.getCache_type() == '1' ? "selected" : ""%>><%=getLocString("acl.cacheType.noCache")%></option>
      <option value='2' <%=bean.getCache_type() == '2' ? "selected" : ""%>><%=getLocString("acl.cacheType.fullCache")%></option>
    </select></div></td>
  </tr>
  </table>
  <table class=list cellspacing=0 cellspadding=0 id=aclAddresses>
  <col width="15%">
  <col width="85%">
  <tr>
  	<th colspan="2"><%=getLocString("acl.addresses")%></th>
  </tr>
  <tr class=row0>
  	<td><input class=txt name="addresses" id="new_address"></td>
  	<td><%addButton(out, "add address", "add address", "acl.addButtonHint", "return addAclAddress()");%></td>
  </tr>
  </table>
</div><%
  page_menu_begin(out);
<<<<<<< edit.jsp
  page_menu_button(session, out, "mbSave",  add ? "Add" : "Save",  add ? "Add new acl" : "Save changes");
  page_menu_button(session, out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
=======
  page_menu_button(session, out, "mbSave",  add ? "common.buttons.add" : "common.buttons.save",  add ? "acl.addNew" : "common.buttons.saveChanges");
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancelHint", "clickCancel()");
>>>>>>> 1.5
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
