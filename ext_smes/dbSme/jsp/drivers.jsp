<%@ page import="ru.novosoft.smsc.jsp.SMSCJspException,  ru.novosoft.smsc.jsp.SMSCErrors,
				 java.util.Set, java.util.Iterator, ru.novosoft.smsc.dbsme.Drivers,
				 ru.novosoft.smsc.util.StringEncoderDecoder, ru.novosoft.smsc.jsp.PageBean"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.dbsme.Drivers" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("dbsme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.RESULT_OK;
	switch(beanResult = bean.process(request))
	{
    case PageBean.RESULT_OK:
    case PageBean.RESULT_ERROR:
      break;
		case Drivers.RESULT_DONE:
		case Drivers.RESULT_OVERVIEW:
			response.sendRedirect("index.jsp");
			return;
		case Drivers.RESULT_PARAMS:
		case Drivers.RESULT_DRIVERS:
		case Drivers.RESULT_PROVIDERS:
		case Drivers.RESULT_PROVIDER:
			response.sendRedirect(bean.getMenuSelection() + ".jsp");
			return;
		default:
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<%@ include file="inc/menu.jsp"%>
<div class=content>
<script type="text/javascript">
function clickAddDriver()
{
	var typeElem   = document.getElementById('dsdriver_new.type');
	var loadupElem = document.getElementById('dsdriver_new.loadup');
    var newRow = drivers_table_id.insertRow(drivers_table_id.rows.length-1);
	newRow.className = "row" + (drivers_table_id.rows.length & 1);
	newRow.id = "dsdriver_row_" + typeElem.value;
	var newCell = document.createElement("td");
	newCell.innerHTML = "<input class=txt name=\"<%=bean.prefix%>." + typeElem.value + ".type\" value=\"" + typeElem.value + "\">";
	newRow.appendChild(newCell);
	newCell = document.createElement("td");
	newCell.innerHTML = "<input class=txtW name=\"<%=bean.prefix%>." + typeElem.value + ".loadup\" value=\"" + loadupElem.value + "\">";
	newRow.appendChild(newCell);
	newCell = document.createElement("td");
	newCell.innerHTML = "<img src=\"/images/but_del.gif\" class=button jbuttonName=\"mbRemove\" jbuttonValue=\"common.buttons.remove\" title=\"dbsme.hint.remove_driver\" jbuttonOnclick=\"return clickRemoveDriver('" + newRow.id + "');\">";
	newRow.appendChild(newCell);

	typeElem.value = "";
	loadupElem.value = "";

	return false;
}
function clickRemoveDriver(id_to_remove)
{
	var rowElem = drivers_table_id.rows(id_to_remove);
	drivers_table_id.deleteRow(rowElem.rowIndex);

	return false;
}
</script>
<table class=list cellspacing=0 id=drivers_table_id>
<col width=1%>
<col width=98%>
<col width=1%>
<tr>
	<th>type</th>
	<th colspan=2>loadup</th>
</tr>
<%
	int rowN = 0;
	Set driverSectionNames = bean.getSectionChildSectionNames(bean.prefix);
	for (Iterator i = driverSectionNames.iterator(); i.hasNext();) {
		String driverSectionName = (String) i.next();
		final String type = bean.getString(driverSectionName + ".type");
		final String loadup = bean.getString(driverSectionName + ".loadup");
		%><tr class=row<%=(rowN++)&1%> id=dsdriver_row_<%=StringEncoderDecoder.encode(type)%>>
			<td><input class=txt name="<%=driverSectionName%>.type"    value="<%=type%>"></td>
			<td><input class=txtW name="<%=driverSectionName%>.loadup" value="<%=loadup%>"></td>
			<td><%button(out, "/images/but_del.gif", "mbDel", "common.buttons.remove", "dbsme.hint.remove_driver", "return clickRemoveDriver('dsdriver_row_" + StringEncoderDecoder.encode(type) + "');");%></td>
		</tr><%
	}
%>
<tr id=dsdriver_new class=row<%=(rowN+1)&1%>>
	<td><input class=txt  id="dsdriver_new.type"   name="dsdriver_new_type"  ></td>
	<td><input class=txtW id="dsdriver_new.loadup" name="dsdriver_new_loadup"></td>
	<td><%button(out, "/images/but_add.gif", "mbAdd", "common.buttons.add", "dbsme.hint.add_new_driver", "return clickAddDriver();");%></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",   "dbsme.hint.accept");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "dbsme.hint.cancel");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>