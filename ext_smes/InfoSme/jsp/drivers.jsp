<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.Drivers,
   			     java.util.Iterator, ru.novosoft.smsc.util.StringEncoderDecoder, java.util.Collection"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Drivers" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<script type="text/javascript">
function clickAddDriver()
{
	var typeElem   = opForm.all.dsdriver_new_type;
	var loadupElem = opForm.all.dsdriver_new_loadup;
    var newRow = drivers_table_id.insertRow(drivers_table_id.rows.length-1);
	newRow.className = "row" + (drivers_table_id.rows.length & 1);
	newRow.id = "dsdriver_row_" + typeElem.value;
	var newCell = document.createElement("td");
	newCell.innerHTML = "<input class=txt name=\"<%=Drivers.DRIVERS_SECTION_NAME%>." + typeElem.value + ".type\" value=\"" + typeElem.value + "\">";
	newRow.appendChild(newCell);
	newCell = document.createElement("td");
	newCell.innerHTML = "<input class=txtW name=\"<%=Drivers.DRIVERS_SECTION_NAME%>." + typeElem.value + ".loadup\" value=\"" + loadupElem.value + "\">";
	newRow.appendChild(newCell);
	newCell = document.createElement("td");
	newCell.innerHTML = "<img src=\"/images/but_del.gif\" class=button jbuttonName=\"mbRemove\" jbuttonValue=\"<%= getLocString("infosme.button.remove_ds_driver")%>\" title=\"<%= getLocString("infosme.hint.remove_ds_driver")%>\" jbuttonOnclick=\"return clickRemoveDriver('" + newRow.id + "');\">";
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
	<th><%= getLocString("infosme.label.type")%></th>
	<th colspan=2><%= getLocString("infosme.label.loadup")%></th>
</tr>
<%
	int rowN = 0;
	Collection driverSectionNames = bean.getDriverSectionNames();
	for (Iterator i = driverSectionNames.iterator(); i.hasNext();) {
		String driverSectionName = (String) i.next();
		final String type = bean.getString(driverSectionName + ".type");
		final String loadup = bean.getString(driverSectionName + ".loadup");
		%><tr class=row<%=(rowN++)&1%> id=dsdriver_row_<%=StringEncoderDecoder.encode(type)%>>
			<td><input class=txt name="<%=driverSectionName%>.type"    value="<%=type%>"></td>
			<td><input class=txtW name="<%=driverSectionName%>.loadup" value="<%=loadup%>"></td>
			<td><%button(out, "but_del.gif", "mbDel", "infosme.button.remove_ds_driver", "infosme.hint.remove_ds_driver", "return clickRemoveDriver('dsdriver_row_" + StringEncoderDecoder.encode(type) + "');");%></td>
		</tr><%
	}
%>
<tr id=dsdriver_new class=row<%=(rowN+1)&1%>>
	<td><input class=txt  id="dsdriver_new_type"   name="dsdriver_new_type"  ></td>
	<td><input class=txtW id="dsdriver_new_loadup" name="dsdriver_new_loadup"></td>
	<td><%button(out, "but_add.gif", "mbAdd", "infosme.button.add_ds_driver", "infosme.hint.add_ds_driver", "return clickAddDriver();");%></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",  "infosme.hint.done_editing");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "infosme.hint.cancel_changes");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>