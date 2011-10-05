/*
 * Copyright (c) 2006, Your Corporation. All Rights Reserved.
 */

var global_counter = 0;
var global_http_counter = 0;
//FF
function removeRow(tbl, rowId) {
//    alert("RR " + tbl + " | " + rowId);
    var tbl = getElementByIdUni(tbl);
    for(var i=0; i<tbl.rows.length; i++){
        if(tbl.rows[i].id==rowId){
            tbl.deleteRow(i);
        }
    }
//    var rowElem = tbl.rows[rowId];
//    tbl.deleteRow(rowElem.rowIndex);
}

//service id
//FF
function addSiteDefaultServiceIdplace(valueElem, selectElem) {
    var valueElem = getElementByIdUni( valueElem );
    var typeSelect = getElementByIdUni( selectElem );
    var selectElem = typeSelect.options[typeSelect.selectedIndex].value;

    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var siteServiceIdPlaceName = valueElem.value;
        var tbl = getElementByIdUni("site_service_id_tbl");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "site_service_id_place_" + "_" + (inner__counter);
        var newCell = document.createElement("td");
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(siteServiceIdPlaceName, inner__counter, selectElem, 'siteServiceIdSelect_');
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "<input type=\"text\" size=\"45\" style=\"color:black;\" value=\"" + siteServiceIdPlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"siteServiceIdName\" value=\"" + siteServiceIdPlaceName + "\">"
                + "<input type=\"hidden\" name=\"siteServiceIdType\" value=\"" + siteServiceIdPlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                + siteServiceIdPlaceName + " Default Service Id place\"  onClick=\"removeRow(\'site_service_id_tbl\', " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';
        newRow.appendChild(newCell);

        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}
//FF
function addAbonDefaultServiceIdplace(valueElem, selectElem) {
    var valueElem = getElementByIdUni( valueElem );
    var typeSelect = getElementByIdUni( selectElem );
    var selectElem = typeSelect.options[typeSelect.selectedIndex].value;

    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var abonServiceIdPlaceName = valueElem.value;
        var tbl = getElementByIdUni("abon_service_id_tbl");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "abon_service_id_place_" + "_" + (inner__counter);
        var newCell = document.createElement("td");
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(abonServiceIdPlaceName, inner__counter, selectElem, 'abonServiceIdSelect_');
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "<input type=\"text\" size=\"45\" style=\"color:black;\" value=\"" + abonServiceIdPlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"abonentServiceIdName\" value=\"" + abonServiceIdPlaceName + "\">"
                + "<input type=\"hidden\" name=\"abonentServiceIdType\" value=\"" + abonServiceIdPlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                + abonServiceIdPlaceName + " Default Service Id place\"  onClick=\"removeRow(\'abon_service_id_tbl\', " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';

        newRow.appendChild(newCell);

        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}

//route id
//FF
function addSiteDefaultRouteIdplace(valueElem, selectElem) {
    var valueElem = getElementByIdUni( valueElem );
    var typeSelect = getElementByIdUni( selectElem );
    var selectElem = typeSelect.options[typeSelect.selectedIndex].value;

    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var siteRouteIdPlaceName = valueElem.value;
        var tbl = getElementByIdUni("site_route_id_tbl");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "site_route_id_place_" + "_" + (inner__counter);
        var newCell = document.createElement("td");
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(siteRouteIdPlaceName, inner__counter, selectElem, 'siteRouteIdSelect_');
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "<input type=\"text\" size=\"45\" style=\"color:black;\" value=\"" + siteRouteIdPlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"siteRouteIdName\" value=\"" + siteRouteIdPlaceName + "\">"
                + "<input type=\"hidden\" name=\"siteRouteIdType\" value=\"" + siteRouteIdPlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                + siteRouteIdPlaceName + " Default Route Id place\"  onClick=\"removeRow(\'site_route_id_tbl\', " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';
        newRow.appendChild(newCell);

        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}
//FF
function addAbonDefaultRouteIdplace(valueElem, selectElem) {
    var valueElem = getElementByIdUni( valueElem );
    var typeSelect = getElementByIdUni( selectElem );
    var selectElem = typeSelect.options[typeSelect.selectedIndex].value;

    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var abonRouteIdPlaceName = valueElem.value;
        var tbl = getElementByIdUni("abon_route_id_tbl");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "abon_route_id_place_" + "_" + (inner__counter);
        var newCell = document.createElement("td");
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(abonRouteIdPlaceName, inner__counter, selectElem, 'abonRouteIdSelect_');
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "<input type=\"text\" size=\"45\" style=\"color:black;\" value=\"" + abonRouteIdPlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"abonentRouteIdName\" value=\"" + abonRouteIdPlaceName + "\">"
                + "<input type=\"hidden\" name=\"abonentRouteIdType\" value=\"" + abonRouteIdPlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                + abonRouteIdPlaceName + " Default Route Id place\"  onClick=\"removeRow(\'abon_route_id_tbl\', " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';

        newRow.appendChild(newCell);

        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}
//FF
function addSiteDefaultUSRplace(valueElem, selectElem) {
    var valueElem = getElementByIdUni( valueElem );
    var typeSelect = getElementByIdUni( selectElem );
    var selectElem = typeSelect.options[typeSelect.selectedIndex].value;

    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var siteUsrPlaceName = valueElem.value;
        var tbl = getElementByIdUni("site_usr_tbl");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "site_usr_place_" + "_" + (inner__counter);
//        var newCell = newRow.insertCell();
        var newCell = document.createElement("td");
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(siteUsrPlaceName, inner__counter, selectElem, 'siteUsrSelect_');
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "<input type=\"text\" size=\"45\" style=\"color:black;\" value=\"" + siteUsrPlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"siteUsrName\" value=\"" + siteUsrPlaceName + "\">"
                + "<input type=\"hidden\" name=\"siteUsrType\" value=\"" + siteUsrPlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                + siteUsrPlaceName + " Default USR place\"  onClick=\"removeRow(\'site_usr_tbl\', " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';
        newRow.appendChild(newCell);

        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}
//FF
function addAbonDefaultUSRplace(valueElem, selectElem) {
    var valueElem = getElementByIdUni( valueElem );
    var typeSelect = getElementByIdUni( selectElem );
    var selectElem = typeSelect.options[typeSelect.selectedIndex].value;
    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var abonUsrPlaceName = valueElem.value;
        var tbl = getElementByIdUni("abon_usr_tbl");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "abon_usr_place_" + "_" + (inner__counter);
        var newCell = document.createElement("td");
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(abonUsrPlaceName, inner__counter, selectElem, 'abonUsrSelect_');
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "<input type=\"text\" size=\"45\" style=\"color:black;\" value=\"" + abonUsrPlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"abonentUsrName\" value=\"" + abonUsrPlaceName + "\">"
                + "<input type=\"hidden\" name=\"abonentUsrType\" value=\"" + abonUsrPlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                + abonUsrPlaceName + " Default USR place\"  onClick=\"removeRow(\'abon_usr_tbl\', " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';

        newRow.appendChild(newCell);

        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}
//FF
function addDefaultAddressPlace(valueElem, selectElem) {
    var valueElem = getElementByIdUni( valueElem );
    var typeSelectAddress = getElementByIdUni( selectElem );
    var selectElem = typeSelectAddress.options[typeSelectAddress.selectedIndex].value;

    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var sitePlaceName = valueElem.value;
        var tbl = getElementByIdUni("site_address_tbl");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "site_place_" + "_" + (inner__counter);
//        var newCell = newRow.insertCell();
        var newCell = document.createElement("td");
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(sitePlaceName, inner__counter, selectElem, 'siteSelect_');
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "<input type=\"text\" size=\"45\" style=\"color:black;\" value=\"" + sitePlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"siteAddrName\" value=\"" + sitePlaceName + "\">"
                + "<input type=\"hidden\" name=\"siteAddrType\" value=\"" + sitePlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                + sitePlaceName + " Default Address place\"  onClick=\"removeRow('site_address_tbl', " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';

        newRow.appendChild(newCell);

        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}
//FF
function addGlobAddressPlace(valueElem, selectElem) {
    var valueElem = getElementByIdUni( valueElem );
    var typeSelectAddress = getElementByIdUni( selectElem );
    var selectElem = typeSelectAddress.options[typeSelectAddress.selectedIndex].value;
    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var addrPlaceName = valueElem.value;
//        var tbl = opForm.all.abon_address_tbl;
        var tbl = getElementByIdUni("abon_address_tbl");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "address_place_" + "_" + (inner__counter);
//        var newCell = newRow.insertCell();
        var newCell = document.createElement("td");
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(addrPlaceName, inner__counter, selectElem, 'addressSelect_');
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
//        newCell = newRow.insertCell();
        newCell = document.createElement("td");
        newCell.innerHTML = "<input type=\"text\" size=\"45\" style=\"color:black;\" value=\"" + addrPlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"abonAddrName\" value=\"" + addrPlaceName + "\">"
                + "<input type=\"hidden\" name=\"abonAddrType\" value=\"" + addrPlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
//        newCell = newRow.insertCell();
        newCell = document.createElement("td");
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
//                + addrPlaceName + " Global Address place\"  onClick=\"removeRow(opForm.all.abon_address_tbl, " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';
                + addrPlaceName + " Global Address place\"  onClick=\"removeRow(\'abon_address_tbl\', " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';

        newRow.appendChild(newCell);

        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}


var selectTypeArr = new Array("url", "param", "header", "cookie");

function getSOptions(selVal) {
    var strSel = "";
    i = 0;
    while (i < selectTypeArr.length) {
        if (selVal == selectTypeArr[i]) {
            strSel += "        <option value=\"" + selectTypeArr[i] + "\" selected=\"true\">" + selectTypeArr[i] + "</option>\n";
        } else {
            strSel += "        <option value=\"" + selectTypeArr[i] + "\">" + selectTypeArr[i] + "</option>\n";
        }
        i++;
    }
    return strSel;
}
function getSelectType(nameElem, counterElem, selVal, selType) {
    var sel_name = selType + nameElem + "_" + (counterElem);
    return ""
            + "<select id=\"" + sel_name + "\"   name=\"" + sel_name + "\"  class=\"txt\">\n" + getSOptions(selVal);
    + "</select>";

}

function trimStr(sString) {
    while (sString.substring(0, 1) == ' ') {
        sString = sString.substring(1, sString.length);
    }
    while (sString.substring(sString.length - 1, sString.length) == ' ') {
        sString = sString.substring(0, sString.length - 1);
    }
    return sString;
}

function commonOnLoad() {
    checkAbonentSubj();
    checkSiteSubj();
}

function toggleVisible(p, c) {
        var c1 = getElementByIdUni(c+"1");
        var c2 = getElementByIdUni(c+"2");
        var c3 = getElementByIdUni(c+"3");
        var c4 = getElementByIdUni(c+"4");
        var c5 = getElementByIdUni(c+"5");

        var p = getElementByIdUni(p);
        var c = getElementByIdUni(c);
        var o = p.className == "collapsing_list_opened";
        p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
//        c.runtimeStyle.display = o ? "none" : "block";
//        c.style.display = o ? "none" : "block";

//        alert( "B c1.style.display=" + c1.style.display + " o=" + o);
        c1.style.display = ( o ? "none" : "block" );
        c2.style.display = ( o ? "none" : "block" );
        c3.style.display = ( o ? "none" : "block" );
        c4.style.display = ( o ? "none" : "block" );
        c5.style.display = ( o ? "none" : "block" );
//        alert( "A c1.style.display=" + c1.style.display);
}

function removeAbonetSubj_(rowId) {
//    var selectElem = opForm.all.abonSubjSelect;
//    var tbl = opForm.all.abonents_table;
    var selectElem = getElementByIdUni("abonSubjSelect");
    var tbl = getElementByIdUni("abonents_table");
    var rowElem = tbl.rows[rowId];
//    var subjValue = rowElem.all.subjAbon.value;
    var subjValue;
    for(var i =0; i<rowElem.cells.length; i++){
        var cell = rowElem.cells[i];
        if(cell.id == 'td_subjAbon'){
            subjValue = cell.innerHTML;
        }
    }
//    alert('td_subjAbon' + subjValue);
    var oOption = document.createElement("OPTION");
    selectElem.options.add(oOption);
//    oOption.innerText = subjValue;
    oOption.text = subjValue;
    oOption.value = subjValue;
    selectElem.disabled = false;
    tbl.deleteRow(rowElem.rowIndex);
}

function removeAbonetSubj(rowId, subjName) {
//    var selectElem = opForm.all.abonSubjSelect;
//    var tbl = opForm.all.abonents_table;
    var selectElem = getElementByIdUni("abonSubjSelect");
    var tbl = getElementByIdUni("abonents_table");
    var rowElem = tbl.rows[rowId];
//    var subjValue = rowElem.all.subjAbon.value;
//    var subjValue ;
//    for(var i =0; i<rowElem.cells.length; i++){
//        var cell = rowElem.cells[i];
//        if(cell.id == 'td_subjAbon'){
//            subjValue = cell.innerHTML;
//        }
//    }
//    alert('td_subjAbon' + subjValue);
    var oOption = document.createElement("OPTION");
    selectElem.options.add(oOption);
//    oOption.innerText = subjValue;
    oOption.text = subjName;
    oOption.value = subjName;
    selectElem.disabled = false;
    tbl.deleteRow(rowElem.rowIndex);
}
//-

function checkAbonentSubj() {
//    var selectElem = opForm.all.abonSubjSelect;
    var selectElem = getElementByIdUni("abonSubjSelect");
    if (selectElem.options.length < 1) {
        selectElem.disabled = true;
    }
}

function checkSiteSubj() {
//    var selectElem = opForm.all.siteSubjSelect;
    var selectElem = getElementByIdUni("siteSubjSelect");
    if (selectElem.options.length < 1) {
        selectElem.disabled = true;
    }
}

function addAbonentSubj() {
//    var selectElem = opForm.all.abonSubjSelect;
    var selectElem = getElementByIdUni("abonSubjSelect");
    if (selectElem.options.length > 0) {
        var subjValue = selectElem.options[selectElem.selectedIndex].value;
//        var tbl = opForm.all.abonents_table;
        var tbl = getElementByIdUni("abonents_table");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "abonRow_" + (global_http_counter++);
        newCell = document.createElement("td");
        newCell.innerHTML = '<img src="content/images/subject.gif">';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = subjValue + '<input id="subjAbon" type="hidden" name="abonSubj" value="' + subjValue + '">';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
//        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeAbonetSubj(\'' + newRow.id + '\');" style="cursor: pointer;">';
        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeAbonetSubj(\'' + newRow.id + '\',\'' + subjValue + '\');" style="cursor: pointer;">';
        newRow.appendChild(newCell);
        selectElem.options[selectElem.selectedIndex] = null;
        selectElem.focus();
    }
    checkAbonentSubj();
}


function addSiteSubj() {
//    var selectElem = opForm.all.siteSubjSelect;
    var selectElem = getElementByIdUni("siteSubjSelect");
    if (selectElem.options.length > 0) {
        var subjValue = selectElem.options[selectElem.selectedIndex].value;
//        var tbl = opForm.all.sites_table;
        var tbl = getElementByIdUni("sites_table");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "siteSubRow_" + (global_http_counter++);
        newCell = document.createElement("td");
        var temp= '<input type="radio" name="defaultSiteObjId" id="'+global_http_counter+'" '+isSiteChecked()+' value="'+ subjValue +'"><img src="content/images/subject.gif" alt="">';
        newCell.innerHTML = temp;
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = subjValue + '<input id="subSite" type="hidden" name="siteSubj" value="' + subjValue + '">';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
//        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeSiteSubj(\'' + newRow.id + '\');" style=cursor: pointer;">';
        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeSiteSubj(\'' + newRow.id + '\',\'' + subjValue + '\');" style="cursor: pointer;">';
        newRow.appendChild(newCell);
        selectElem.options[selectElem.selectedIndex] = null;
        selectElem.focus();
    }
    checkSiteSubj();
}

function removeSiteSubj(rowId, subjValue) {
    var defaultSiteRadios = document.getElementsByName("defaultSiteObjId");
    var siteSubRow="siteSubRow_";
    var _rowId=rowId.substr(siteSubRow.length, rowId.length);
    if (defaultSiteRadios.length>1)
    for (var i=0; i<defaultSiteRadios.length;i++ ) {
        if (defaultSiteRadios[i].checked && (_rowId == defaultSiteRadios[i].value || (""+(parseInt(_rowId)+1)) == defaultSiteRadios[i].id) ) {
          alert("Couldn't remove default site subject");
          return false;
        }
    }
//    var selectElem = opForm.all.siteSubjSelect;
    var selectElem = getElementByIdUni("siteSubjSelect");
//    var tbl = opForm.all.sites_table;
    var tbl = getElementByIdUni("sites_table");
//    var rowElem = tbl.rows(rowId);
    var rowElem = tbl.rows[rowId];
//    var subjValue = rowElem.all.subSite.value;
    var oOption = document.createElement("OPTION");
    selectElem.options.add(oOption);
//    oOption.innerText = subjValue;
    oOption.text = subjValue;
    oOption.value = subjValue;
    selectElem.disabled = false;
    tbl.deleteRow(rowElem.rowIndex);
}

function removeSiteSubj_(rowId) {
    var defaultSiteRadios = document.getElementsByName("defaultSiteObjId");
    var siteSubRow="siteSubRow_";
    var _rowId=rowId.substr(siteSubRow.length, rowId.length);
    if (defaultSiteRadios.length>1)
    for (var i=0; i<defaultSiteRadios.length;i++ ) {
        if (defaultSiteRadios[i].checked && (_rowId == defaultSiteRadios[i].value || (""+(parseInt(_rowId)+1)) == defaultSiteRadios[i].id) ) {
          alert("Couldn't remove default site subject");
          return false;
        }
    }
//    var selectElem = opForm.all.siteSubjSelect;
    var selectElem = getElementByIdUni("siteSubjSelect");
//    var tbl = opForm.all.sites_table;
    var tbl = getElementByIdUni("sites_table");
    var rowElem = tbl.rows(rowId);
    var subjValue = rowElem.all.subSite.value;
    var oOption = document.createElement("OPTION");
    selectElem.options.add(oOption);
    oOption.innerText = subjValue;
    oOption.value = subjValue;
    selectElem.disabled = false;
    tbl.deleteRow(rowElem.rowIndex);
}

function addAbonentMask(valueElem) {
    var valueElem = getElementByIdUni(valueElem);
    if (validateField(valueElem)) {
//        var tbl = opForm.all.abonents_table;
        var tbl = getElementByIdUni("abonents_table");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "abonRow_" + (global_http_counter++);
        newCell = document.createElement("td");
        newCell.innerHTML = '<img src="content/images/mask.gif">';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = valueElem.value + '<input type="hidden" name="abonAddress" value="' + valueElem.value + '">';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
//        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(opForm.all.abonents_table, \'' + newRow.id + '\')" style="cursor: pointer;">';
        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(\'abonents_table\', \'' + newRow.id + '\')" style="cursor: pointer;">';
        newRow.appendChild(newCell);
        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}

function addPath(valueElem, prefElem, tblElem, tableName) {
    var valueElem = getElementByIdUni(valueElem);
    var tblElem = getElementByIdUni(tblElem);
    if (trimStr(valueElem.value).length > 0) {
        var tbl = tblElem;
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "pathRow_" + (global_http_counter++);
        newCell = document.createElement("td");
        newCell.innerHTML = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;' + valueElem.value + '<input type="hidden" name="pathLinks" value="' + prefElem + '^' + valueElem.value + '">';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
//        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(opForm.all.sitesp_table_' + tableName + ', \'' + newRow.id + '\')" style="cursor: pointer;">';
        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(\'sitesp_table_' + tableName + '\', \'' + newRow.id + '\')" style="cursor: pointer;">';
        newRow.appendChild(newCell);
        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}

function createImgButton(imgUrl, onclickT, tooltipText) {
    return "<img src=\"" + imgUrl + "\" onclick=\"" + onclickT + "\" title=\"" + tooltipText + "\">";
}

function removeSection_(sectionName) {
    var defaultSiteRadios = document.getElementsByName("defaultSiteObjId");
    var sN = sectionName.replace(/_/g,'.');
    if (defaultSiteRadios.length>1)
    for (var i=0; i<defaultSiteRadios.length;i++ ) {
        if (trimStr(defaultSiteRadios[i].value) == sN && defaultSiteRadios[i].checked) {
          alert("Couldn't remove default site");
          return false;
        }
    }
    sectionElem = document.getElementById("sectionHeader_" + sectionName);
//    alert(sectionElem);

      table = getElementByIdUni( "div_site_table" );
//      var rowElem = table.rows[rowId];
      var rowElem = table.rows["sectionHeader_" + sectionName];
      table.deleteRow(rowElem.rowIndex);

//    sectionElem.removeNode(true);
//    sectionElem = document.getElementById("sectionValue_" + sectionName);
//    if (sectionElem) sectionElem.removeNode(true);
}

function removeSection(sectionName, rowId, tblId) {
    var defaultSiteRadios = document.getElementsByName("defaultSiteObjId");
    var sN = sectionName.replace(/_/g,'.');
    if (defaultSiteRadios.length>1)
    for (var i=0; i<defaultSiteRadios.length;i++ ) {
        if (trimStr(defaultSiteRadios[i].value) == sN && defaultSiteRadios[i].checked) {
          alert("Couldn't remove default site");
          return false;
        }
    }
    sectionElem = document.getElementById("sectionHeader_" + sectionName);
//    alert( "removeSection " + sectionElem);

      table = getElementByIdUni( tblId );
//      var rowElem = table.rows[rowId];
      var rowElem = table.rows[rowId];
      table.deleteRow(rowElem.rowIndex);
}

function addSite(valueElem, portElem, addPathStr, msgVal, msgVal1) {
//    alert("addSite");
    var valueElem = getElementByIdUni(valueElem);
    var portElem = getElementByIdUni(portElem);
    var s = trim(valueElem.value)+"_"+trimStr(portElem.value);
    console.log(s);
    if(document.getElementById(trim(valueElem.value)+"_"+trimStr(portElem.value)) != null){
        alert("! " + msgVal);
        return false;
    }
    if (!trimStr(portElem.value).length > 0)
        portElem.value = '80';
    if ((trimStr(valueElem.value).length > 0) && validateField(portElem)) {

        var siteName = valueElem.value.replace(/\./g,'_');

//        var tbl = opForm.all.div_site_table;
        var tbl = getElementByIdUni("div_site_table");
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.id = "sectionHeader_"+siteName+"_"+trimStr(portElem.value);
//        newCell = newRow.insertCell();
        newCell = document.createElement("TD");
//        newCell.innerHTML = addSiteSectionBody(siteName, trimStr(valueElem.value), trimStr(portElem.value), addPathStr);
        newCell.innerHTML = addSiteSectionBody(siteName, trimStr(valueElem.value), trimStr(portElem.value), addPathStr, div_site_table);
        newRow.appendChild(newCell);
        valueElem.value = "";
        portElem.value = '80';
        valueElem.focus();
        return true;
    } else
        return false;
}

function addSiteSectionBody_(siteName, siteFullName, sitePort, addPathStr) {
    var sectionHeader_ = "sectionHeader_" + siteName;
    var newPSN = getElementByIdUni( "newPath_" + "siteName" );
    var sitespTSN = getElementByIdUni( "sitesp_table_" + "siteName" );
    var rowId = "sectionHeader_" + siteName;
    return ""
//            + "<div class=\"collapsing_tree_opened\" id=\"" + sectionHeader_ + "\" style=\"width:100%;background-image:none\">"// onclick=\"collasping_tree_showhide_section('" + siteName + "')\">"
            + " <table id=\"sitesp_table_" + siteName + "\" cellpadding=\"0\" cellspacing=\"0\" class=\"properties_list\">"
            + " <col width=\"1%\">"
            + " <col width=\"99%\">"
            + " <tr>"
            + " <td width=\"100%\"><input type=\"radio\" name=\"defaultSiteObjId\" " +isSiteChecked() + " value=\"" + siteFullName + "\">&nbsp;" + siteFullName +":"+sitePort+ "</td>"
            + " <td align=\"left\" nowrap=\"true\"><input type=\"hidden\" name=\"sitesHost\" id=\"" + siteFullName + "\"  value=\"" + siteFullName + "\">"
            + " <input type=\"hidden\" name=\"sitesPort\" value=\"" + siteFullName + "_" + sitePort + "\"></td>"
            + " <td><img src=\"content/images/but_del.gif\" onClick=\"removeSection('" + siteName + ')\"></td>"
//            + " <td><img src=\"content/images/but_del.gif\" onClick=\"removeSection('" + siteName + "','sectionHeader_" + siteName + "','div_site_table' )\"></td>"
//            + " <td><img src=\"content/images/but_del.gif\" onClick=\"removeSection('" + siteName + "','" + rowId + "','div_site_table' )\"></td>"
            + " </tr>"
            + " <tr>"
            + " <td nowrap=\"true\" align=\"right\">" + addPathStr + "&nbsp;</td>"
            + " <td align=\"right\"><input id=\"newPath_" + siteName + "\" class=\"txt\" name=\"newPath_" + siteName + "\" onkeyup=\"resetValidation(this)\"></td>"
            + " <td><img src=\"content/images/but_add.gif\" alt=\"" + addPathStr + "\" "
//            + "onclick=\"addPath(opForm.all.newPath_" + siteName + ", '" + siteFullName + "', opForm.all.sitesp_table_" + siteName + ", '" + siteName + "')\" "
            + "onclick=\"addPath('newPath_" + siteName + "', '" + siteFullName + "', 'sitesp_table_" + siteName + "', '" + siteName + "')\" "
            + "style=\"cursor:pointer;\"></td>"
            + " </tr>"
            + " </table>"
//            + "</div>";
}

function addSiteSectionBody(siteName, siteFullName, sitePort, addPathStr, tblId) {
    var sectionHeader_ = "sectionHeader_" + siteName + "_"+sitePort;
    var newPSN = getElementByIdUni( "newPath_" + "siteName");
    var sitespTSN = getElementByIdUni( "sitesp_table_" + "siteName");
    var rowId = "sectionHeader_" + siteName+"_"+sitePort;
    return ""
//            + "<div class=\"collapsing_tree_opened\" id=\"" + sectionHeader_ + "\" style=\"width:100%;background-image:none\">"// onclick=\"collasping_tree_showhide_section('" + siteName + "')\">"
            + " <table id=\"sitesp_table_" + siteName +"_"+ sitePort + "\" cellpadding=\"0\" cellspacing=\"0\" class=\"properties_list\">"
            + " <col width=\"1%\">"
            + " <col width=\"99%\">"
            + " <tr>"
            + " <td width=\"100%\"><input type=\"radio\" name=\"defaultSiteObjId\" " +isSiteChecked() + " value=\"" + siteFullName+"_"+sitePort + "\">&nbsp;" + siteFullName +":"+sitePort+ "</td>"
            + " <td align=\"left\" nowrap=\"true\"><input type=\"hidden\" name=\"sitesHost\" id=\"" + siteFullName + "_" + sitePort + "\"  value=\"" + siteFullName + "\">"
            + " <input type=\"hidden\" name=\"sitesPort\" value=\"" + siteFullName + "_" + sitePort + "\"></td>"
//            + " <td><img src=\"content/images/but_del.gif\" onClick=\"removeSection('" + siteName + "')\"></td>"
//            + " <td><img src=\"content/images/but_del.gif\" onClick=\"removeSection('" + siteName + "','sectionHeader_" + siteName + "','div_site_table' )\"></td>"
            + " <td><img src=\"content/images/but_del.gif\" onClick=\"removeSection('" + siteName + "_"+sitePort + "','" + rowId + "','div_site_table' )\" style=\"cursor:pointer;\"></td>"
            + " </tr>"
            + " <tr>"
            + " <td nowrap=\"true\" align=\"right\">" + addPathStr + "&nbsp;</td>"
            + " <td align=\"right\"><input id=\"newPath_" + siteName + "_" + sitePort + "\" class=\"txt\" name=\"newPath_" + siteName + "_" + sitePort + "\" onkeyup=\"resetValidation(this)\"></td>"
            + " <td><img src=\"content/images/but_add.gif\" alt=\"" + addPathStr + "\" "
//            + "onclick=\"addPath(opForm.all.newPath_" + siteName + ", '" + siteFullName + "', opForm.all.sitesp_table_" + siteName + ", '" + siteName + "')\" "
            + "onclick=\"addPath('newPath_" + siteName + "_" + sitePort + "', '" + siteFullName + "_" + sitePort + "', 'sitesp_table_" + siteName + "_"+ sitePort + "', '" + siteName + "_" + sitePort + "')\" "
            + "style=\"cursor:pointer;\"></td>"
            + " </tr>"
            + " </table>"
//            + "</div>";
}

function isSiteChecked() {
  var defaultSiteRadios = document.getElementsByName("defaultSiteObjId");
  if (defaultSiteRadios.length==0)
    return "checked";
  else
    return "";
}

function validateHttpRoute() {
   var div_sections = document.getElementById("div_site_table").getElementsByTagName("div");
   if (div_sections && div_sections.length>0) {
     return checkPathesForHosts(div_sections);
   }
   return true;
}

function validateHttpSubj() {
  var rows = document.getElementById("sources_table").getElementsByTagName("tr");
  if (!rows || rows.length == 0) {
    alert('<fmt:message>error.routing.subjects.http_subject_addresses_not_exists</fmt:message>');
    return false;
  }
  return true;
}

function validateHttpSite() {
//  var div_sections = document.getElementById("div_site_table").getElementsByTagName("div");
//  var div_sections = document.getElementById("div_site_table").getElementsByTagName("div");
  var div_sections = document.getElementById("div_site_table").getElementsByTagName("table");
  if (!div_sections || div_sections.length==0) {
    alert('<fmt:message>error.routing.sites.http_site_sites_not_exists</fmt:message>');
    return false;
  }
  return checkPathesForHosts(div_sections);
}

function checkPathesForHosts(div_sections) {
  var trs;
  var host_name;
  var input;
  for (var i=0;i<div_sections.length;i++) {
    trs = div_sections[i].getElementsByTagName("tr");
    if (!trs || trs.length==2) {
      host_name = getHostNameForSection(div_sections[i]);
      alert('<fmt:message>error.routing.sites.http_site_pathes_not_exists</fmt:message>'+host_name);
      input = trs[1].getElementsByTagName("input")[0];
      openSitesList();
      return validationError(input, '<fmt:message>error.routing.sites.http_site_pathes_not_exists</fmt:message>');
    }
  }
  return true;
}

function getHostNameForSection(section) {
 var sec_id = section.id; //sectionHeader_xxx
 var prefix_length = "sectionHeader_".length;
 return ' ' +sec_id.substring(prefix_length,sec_id.length);
}

function openSitesList() {
//  var dt_sec = opForm.all["collapsing_tree_Sites List"];
  var dt_sec = getElementByIdUni("collapsing_tree_Sites List");
  if (dt_sec.className=="opened")
    return;
  dt_sec.click();
}
