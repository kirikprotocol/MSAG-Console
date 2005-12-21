<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.acl.AclInfo,
                 ru.novosoft.smsc.jsp.smsc.acl.Index"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.acl.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("acl.title");
switch(bean.process(request))
{
	case Index.RESULT_DONE:
    if (bean.getLookupedAcl() == -1)
      response.sendRedirect("index.jsp");
    else
      response.sendRedirect("index.jsp?lookupedAcl="+bean.getLookupedAcl());
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
  <%=getLocString("acl.accessControlList")%>:
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
  page_menu_button(session, out, "mbLookup",  "acl.lookup",  "acl.lookup");
  page_menu_button(session, out, "mbAddAcl",  "acl.add",     "acl.addHint");
  page_menu_button(session, out, "mbDelAcl",  "acl.delete",  "acl.deleteHint");
  page_menu_space(out);
  page_menu_end(out);
  if (aclInfo != null)
  {
    %><input type="hidden" name=lookupedAcl value=<%=aclInfo.getId()%>>
    <div class="content">
      <div class="page_subtitle"><%=getLocString("acl.subTitle")%></div>
      <table class=list cellspacing=0 cellspadding=0>
      <col width="15%">
      <col width="85%">
      <tr class=row0>
        <td><%=getLocString("acl.id")%></td>
        <td><%=aclInfo.getId()%></td>
      </tr>
      <tr class=row1>
        <td><%=getLocString("acl.name")%></td>
        <td><input class=txt name="name" value="<%=aclInfo.getName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
      </tr>
      <tr class=row0>
        <td><%=getLocString("acl.description")%></td>
        <td><input class=txt name="description" value="<%=aclInfo.getDescription()%>"></td>
      </tr>
      <tr class=row1>
        <td><%=getLocString("acl.cacheType")%></td>
        <td><div class=select><select class=txt name="cache_type">
          <option value='0' <%=aclInfo.getCache_mode() == '0' ? "selected" : ""%>><%=getLocString("acl.cacheType.unknown")%></option>
          <option value='1' <%=aclInfo.getCache_mode() == '1' ? "selected" : ""%>><%=getLocString("acl.cacheType.noCache")%></option>
          <option value='2' <%=aclInfo.getCache_mode() == '2' ? "selected" : ""%>><%=getLocString("acl.cacheType.fullCache")%></option>
        </select></div></td>
      </tr>
      </table>
      <hr>
      <%=getLocString("acl.addressesPrefix")%> <input class=txt name=prefix value="<%=bean.getPrefix()%>">
    </div><%
    page_menu_begin(out);
    page_menu_button(session, out, "mbLookupAddresses",  "acl.lookupAddresses", "acl.lookupAddressesHint");
    page_menu_space(out);
    page_menu_end(out);
    %><div class="content">
    <script>
    function delAclAddress(rowN)
    {
      var tbl = document.getElementById('addresses_table');
      var row = tbl.rows("address_row_"+rowN);
      if (confirm('<%=getLocString("acl.delAddressConfirm")%> "' + row.address + '"?')) {
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
      var valueElem = document.getElementById('new_address_qweqwe');
      var addr = valueElem.value;
      if (addr != null && addr.length > 0) {
        var tbl = document.getElementById('addresses_table');
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length) & 1);
        newRow.id = "addrRow_" + (global_counter);
        newCell = document.createElement("td");
        newCell.innerHTML = addr + "<input type=hidden name=new_address id=new_address_" + global_counter + " value=\"" + addr + "\">";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = '<img src="/images/but_del.gif" onClick="removeRow(document.getElementById(\'addresses_table\'), \'' + newRow.id + '\')" style="cursor: hand;">';
        newRow.appendChild(newCell);
        valueElem.value = "";
        valueElem.focus();
        global_counter++;
        return false;
      } else
        return false;
    }
    </script>
    <table class=list cellpadding="0" cellspacing="0" id="addresses_table">
    <col width="200px">
    <th><%=getLocString("acl.address")%></th>
    <th>&nbsp;</th>
    <%
    int row = 0;
    if (bean.getAddresses() != null) {
      for (Iterator i = bean.getAddresses().iterator(); i.hasNext();) {
        String address = (String) i.next();
        %><tr class=row<%=row%2%> id=address_row_<%=row%> address="<%=address%>">
          <td><%=address%></td>
          <td><%delButton(out, "remove address", "remove address", "acl.delButtonHint", "delAclAddress("+row+")");%></td>
        </tr><%
        row++;
      }
    }
    %>
      <tr class=row<%=row++%2%>>
        <td><input class=txt name=new_address id=new_address_qweqwe></td>
        <td><%addButton(out, "add address", "add address", "acl.addButtonHint", "addAclAddress()");%></td>
      </tr>
    </table></div><%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave",  "common.buttons.saveChanges", "common.buttons.saveChanges");
    page_menu_space(out);
    page_menu_end(out);
  }
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
