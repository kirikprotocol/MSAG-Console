<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.acl.Index,
                 ru.novosoft.smsc.jsp.*,
                 ru.novosoft.smsc.admin.acl.AclInfo,
                 ru.novosoft.smsc.admin.route.Mask"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.acl.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Access Control Lists";
switch(bean.process(request))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		break;
	case Index.RESULT_ERROR:
		break;
	case Index.RESULT_ADD_ACL:
		response.sendRedirect("edit.jsp?add=true");
		return;
/*
	case Index.RESULT_EDIT:
		response.sendRedirect("edit.jsp?aclId="+bean.getAclId());
		return;
*/
	default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ACL";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
  Access Control List:
  <select name="aclId">
  <%
    final AclInfo aclInfo = bean.getAclInfo();
    for (Iterator i = bean.getAclNames().iterator(); i.hasNext();) {
      AclInfo acl = (AclInfo) i.next();
      %><option value="<%=acl.getId()%>"<%= aclInfo != null && bean.getAclId() == acl.getId() ? " selected" : ""%>><%=acl.getName()%></option><%
    }
  %>
  </select>
</div><%
  page_menu_begin(out);
  page_menu_button(out, "mbLookup",  "Lookup ACL",  "Lookup ACL");
  page_menu_button(out, "mbAddAcl",  "Add ACL",     "Create new ACL");
  page_menu_button(out, "mbDelAcl",  "Delete ACL",  "Delete selected ACL");
  page_menu_space(out);
  page_menu_end(out);
  if (aclInfo != null)
  {
    %><input type="hidden" name=lookupedAcl value=<%=aclInfo.getId()%>>
    <div class="content">
      <div class="page_subtitle">Access Control List</div>
      <table class=list cellspacing=0 cellspadding=0>
      <col width="15%">
      <col width="85%">
      <tr class=row0>
        <td>id</td>
        <td><%=aclInfo.getId()%></td>
      </tr>
      <tr class=row1>
        <td>name</td>
        <td><input class=txt name="name" value="<%=aclInfo.getName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
      </tr>
      <tr class=row0>
        <td>description</td>
        <td><input class=txt name="description" value="<%=aclInfo.getDescription()%>"></td>
      </tr>
      </table>
      Addresses prefix <input class=txt name=prefix value="<%=bean.getPrefix()%>">
    </div><%
    page_menu_begin(out);
    page_menu_button(out, "mbLookupAddresses",  "Lookup addresses",  "Lookup ACL addresses");
    if (bean.getAddresses() != null) {
      page_menu_space(out);
      page_menu_end(out);
      %><div class="content">
      <script>
      function delAclAddress(rowN)
      {
        var tbl = opForm.all.addresses_table;
        var row = tbl.rows("address_row_"+rowN);
        if (confirm('Are you sure to remove address "' + row.address + '"?')) {
          var addressElem = document.createElement("input");
          addressElem.type = "hidden";
          addressElem.name = "deleted_address";
          addressElem.value = row.address;
          opForm.appendChild(addressElem);
          tbl.deleteRow(row.rowIndex);
        }
        return false;
      }

      var global_counter = 0;
      function addAclAddress()
      {
        var valueElem = opForm.all.new_address;
        var addr = valueElem.value;
        if (addr != null && addr.length > 0) {
          var tbl = opForm.all.addresses_table;
          var newRow = tbl.insertRow(tbl.rows.length);
          newRow.className = "row" + ((tbl.rows.length) & 1);
          newRow.id = "addrRow_" + (global_counter++);
          newCell = document.createElement("td");
          newCell.innerHTML = addr + "<input type=hidden name=new_address value=\"" + addr + "\">";
          newRow.appendChild(newCell);
          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="/images/but_del.gif" onClick="removeRow(opForm.all.addresses_table, \'' + newRow.id + '\')" style="cursor: hand;">';
          newRow.appendChild(newCell);
          valueElem.value = "";
          valueElem.focus();
          return false;
        } else
          return false;
      }
      </script>
      <table class=list cellpadding="0" cellspacing="0" id="addresses_table">
      <col width="200px">
      <th>address</th>
      <th>&nbsp;</th>
      <%
      int row = 0;
      for (Iterator i = bean.getAddresses().iterator(); i.hasNext();) {
        String address = (String) i.next();
        %><tr class=row<%=row%2%> id=address_row_<%=row%> address="<%=address%>">
          <td><%=address%></td>
          <td><%delButton(out, "remove address", "remove address", "remove address", "return delAclAddress("+row+")");%></td>
        </tr><%
        row++;
      }
      %>
        <tr class=row<%=row++%2%>>
          <td><input class=txt name=new_address id=new_address></td>
          <td><%addButton(out, "add address", "add address", "add address", "return addAclAddress()");%></td>
        </tr>
      </table></div><%
      page_menu_begin(out);
    }
    page_menu_button(out, "mbSave",  "Save changes",  "Save changes");
    page_menu_space(out);
    page_menu_end(out);
  }
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
