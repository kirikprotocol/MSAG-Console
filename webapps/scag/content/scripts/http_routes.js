/*
 * Copyright (c) 2006, Your Corporation. All Rights Reserved.
 */

var global_counter = 0;
var global_http_counter = 0;

function removeRow(tbl, rowId) {
    var rowElem = tbl.rows(rowId);
    tbl.deleteRow(rowElem.rowIndex);
}

function addSiteDefaultUSRplace(valueElem, selectElem) {
    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var siteUsrPlaceName = valueElem.value;
        var tbl = opForm.all.site_usr_tbl;
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "site_usr_place_" + "_" + (inner__counter);
        var newCell = newRow.insertCell();
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(siteUsrPlaceName, inner__counter, selectElem, 'siteUsrSelect_');
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.innerHTML = "<input type=\"text\" size=\"50\" style=\"font:bold; color:black;\" value=\"" + siteUsrPlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"siteUsrName\" value=\"" + siteUsrPlaceName + "\">"
                + "<input type=\"hidden\" name=\"siteUsrType\" value=\"" + siteUsrPlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                + siteUsrPlaceName + " Default USR place\"  onClick=\"removeRow(opForm.all.site_usr_tbl, " + "\'" + newRow.id + '\');\" style=\"cursor:hand;\"/>';
        newRow.appendChild(newCell);

        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}

function addAbonDefaultUSRplace(valueElem, selectElem) {
    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var abonUsrPlaceName = valueElem.value;
        var tbl = opForm.all.abon_usr_tbl;
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "abon_usr_place_" + "_" + (inner__counter);
        var newCell = newRow.insertCell();
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(abonUsrPlaceName, inner__counter, selectElem, 'abonUsrSelect_');
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.innerHTML = "<input type=\"text\" size=\"50\" style=\"font:bold; color:black;\" value=\"" + abonUsrPlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"abonentUsrName\" value=\"" + abonUsrPlaceName + "\">"
                + "<input type=\"hidden\" name=\"abonentUsrType\" value=\"" + abonUsrPlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                + abonUsrPlaceName + " Default USR place\"  onClick=\"removeRow(opForm.all.abon_usr_tbl, " + "\'" + newRow.id + '\');\" style=\"cursor:hand;\"/>';

        newRow.appendChild(newCell);

        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}
function addDefaultAddressPlace(valueElem, selectElem) {
    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var sitePlaceName = valueElem.value;
        var tbl = opForm.all.site_address_tbl;
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "site_place_" + "_" + (inner__counter);
        var newCell = newRow.insertCell();
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(sitePlaceName, inner__counter, selectElem, 'siteSelect_');
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.innerHTML = "<input type=\"text\" size=\"50\" style=\"font:bold; color:black;\" value=\"" + sitePlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"siteAddrName\" value=\"" + sitePlaceName + "\">"
                + "<input type=\"hidden\" name=\"siteAddrType\" value=\"" + sitePlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                + sitePlaceName + " Default Address place\"  onClick=\"removeRow(opForm.all.site_address_tbl, " + "\'" + newRow.id + '\');\" style=\"cursor:hand;\"/>';

        newRow.appendChild(newCell);

        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}

function addGlobAddressPlace(valueElem, selectElem) {

    if ((trimStr(valueElem.value).length > 0)) {
        var inner__counter = global_counter++;
        var addrPlaceName = valueElem.value;
        var tbl = opForm.all.abon_address_tbl;
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "address_place_" + "_" + (inner__counter);
        var newCell = newRow.insertCell();
        newCell.width = "7%";
        newCell.innerHTML = getSelectType(addrPlaceName, inner__counter, selectElem, 'addressSelect_');
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.innerHTML = "&nbsp;";
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.innerHTML = "<input type=\"text\" size=\"50\" style=\"font:bold; color:black;\" value=\"" + addrPlaceName + "\" readonly=\"true\"/>"
                + "<input type=\"hidden\" name=\"abonAddrName\" value=\"" + addrPlaceName + "\">"
                + "<input type=\"hidden\" name=\"abonAddrType\" value=\"" + addrPlaceName + "_" + inner__counter + "\">";
        newRow.appendChild(newCell);
        newCell = newRow.insertCell();
        newCell.width = "65%";
        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                + addrPlaceName + " Global Address place\"  onClick=\"removeRow(opForm.all.abon_address_tbl, " + "\'" + newRow.id + '\');\" style=\"cursor:hand;\"/>';

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
        var o = p.className == "collapsing_list_opened";
        p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
        c.runtimeStyle.display = o ? "none" : "block";
}

function removeAbonetSubj(rowId) {
    var selectElem = opForm.all.abonSubjSelect;
    var tbl = opForm.all.abonents_table;
    var rowElem = tbl.rows(rowId);
    var subjValue = rowElem.all.subjAbon.value;
    var oOption = document.createElement("OPTION");
    selectElem.options.add(oOption);
    oOption.innerText = subjValue;
    oOption.value = subjValue;
    selectElem.disabled = false;
    tbl.deleteRow(rowElem.rowIndex);
}

function removeSiteSubj(rowId) {
    var selectElem = opForm.all.siteSubjSelect;
    var tbl = opForm.all.sites_table;
    var rowElem = tbl.rows(rowId);
    var subjValue = rowElem.all.subSite.value;
    var oOption = document.createElement("OPTION");
    selectElem.options.add(oOption);
    oOption.innerText = subjValue;
    oOption.value = subjValue;
    selectElem.disabled = false;
    tbl.deleteRow(rowElem.rowIndex);
}

function checkAbonentSubj() {
    var selectElem = opForm.all.abonSubjSelect;
    if (selectElem.options.length < 1) {
        selectElem.disabled = true;
    }
}

function checkSiteSubj() {
    var selectElem = opForm.all.siteSubjSelect;
    if (selectElem.options.length < 1) {
        selectElem.disabled = true;
    }
}

function addAbonentSubj() {
    var selectElem = opForm.all.abonSubjSelect;
    if (selectElem.options.length > 0) {
        var subjValue = selectElem.options[selectElem.selectedIndex].value;
        var tbl = opForm.all.abonents_table;
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
        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeAbonetSubj(\'' + newRow.id + '\');" style="cursor: hand;">';
        newRow.appendChild(newCell);
        selectElem.options[selectElem.selectedIndex] = null;
        selectElem.focus();
    }
    checkAbonentSubj();
}


function addSiteSubj() {
    var selectElem = opForm.all.siteSubjSelect;
    if (selectElem.options.length > 0) {
        var subjValue = selectElem.options[selectElem.selectedIndex].value;
        var tbl = opForm.all.sites_table;
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "siteSubRow_" + (global_http_counter++);
        newCell = document.createElement("td");
        newCell.innerHTML = '<img src="content/images/subject.gif"><img src="content/images/subject.gif">';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = subjValue + '<input id="subSite" type="hidden" name="siteSubj" value="' + subjValue + '">';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeSiteSubj(\'' + newRow.id + '\');" style=cursor: hand;">';
        newRow.appendChild(newCell);
        selectElem.options[selectElem.selectedIndex] = null;
        selectElem.focus();
    }
    checkSiteSubj();
}

function addAbonentMask(valueElem) {
    if (validateField(valueElem)) {
        var tbl = opForm.all.abonents_table;
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "abonRow_" + (global_http_counter++);
        newCell = document.createElement("td");
        newCell.innerHTML = '<img src="content/images/subject.gif">';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = valueElem.value + '<input type="hidden" name="abonAddress" value="' + valueElem.value + '">';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(opForm.all.abonents_table, \'' + newRow.id + '\')" style="cursor: hand;">';
        newRow.appendChild(newCell);
        valueElem.value = "";
        valueElem.focus();
        return true;
    } else
        return false;
}

function addPath(valueElem, prefElem, tblElem, tableName) {
    if (trimStr(valueElem.value).length > 0) {
        var tbl = tblElem;
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "pathRow_" + (global_http_counter++);
        newCell = document.createElement("td");
        newCell.innerHTML = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;' + valueElem.value + '<input type="hidden" name="pathLinks" value="' + prefElem + '_' + valueElem.value + '">';
        newRow.appendChild(newCell);
        newCell = document.createElement("td");
        newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(opForm.all.sitesp_table_' + tableName + ', \'' + newRow.id + '\')" style="cursor: hand;">';
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

function removeSection(sectionName) {
    sectionElem = document.getElementById("sectionHeader_" + sectionName);
    sectionElem.removeNode(true);
    sectionElem = document.getElementById("sectionValue_" + sectionName);
    sectionElem.removeNode(true);
}

function addSite(valueElem, portElem, addPathStr, msgVal) {
    if(document.getElementById(trim(valueElem.value)) != null){
        alert(msgVal);
        return false;
    }
    if (!trimStr(portElem.value).length > 0)
        portElem.value = '80';
    if ((trimStr(valueElem.value).length > 0) && validateField(portElem)) {

        var siteName = trimStr(valueElem.value.substr(0, valueElem.value.indexOf('.')));

        var tbl = opForm.all.div_site_table;
        var newRow = tbl.insertRow(tbl.rows.length);
        newCell = newRow.insertCell();
        newCell.innerHTML = addSiteSectionBody(siteName, trimStr(valueElem.value), trimStr(portElem.value), addPathStr);
        newRow.appendChild(newCell);
        valueElem.value = "";
        portElem.value = '80';
        valueElem.focus();
        return true;
    } else
        return false;
}

function addSiteSectionBody(siteName, siteFullName, sitePort, addPathStr) {
    var sectionHeader_ = "sectionHeader_" + siteName;
    return ""
            + "<div class=\"collapsing_tree_opened\" id=\"" + sectionHeader_ + "\">"// onclick=\"collasping_tree_showhide_section('" + siteName + "')\">"
            + " <table id=\"sitesp_table_" + siteName + "\" cellpadding=\"0\" cellspacing=\"0\" class=\"properties_list\">"
            + " <col width=\"1%\">"
            + " <col width=\"99%\">"
            + " <tr>"
            + " <td width=\"100%\">" + siteFullName + "</td>"
            + " <td align=\"left\" nowrap=\"true\"><input type=\"hidden\" name=\"sitesHost\" id=\"" + siteFullName + "\"  value=\"" + siteFullName + "\">"
            + " <input type=\"hidden\" name=\"sitesPort\" value=\"" + siteFullName + "_" + sitePort + "\"></td>"
            + " <td><img src=\"content/images/but_del.gif\" onClick=\"removeSection('" + siteName + "')\"></td>"
            + " </tr>"
            + " <tr>"
            + " <td nowrap=\"true\">" + addPathStr + "&nbsp;</td>"
            + " <td align=\"right\"><input id=\"" + siteName + "_newPath\" class=\"txt\" name=\"" + siteName + "_newPath\"></td>"
            + " <td><img src=\"content/images/but_add.gif\" alt=\"" + addPathStr + "\" "
            + "onclick=\"addPath(opForm.all." + siteName + "_newPath, '" + siteFullName + "', opForm.all.sitesp_table_" + siteName + ", '" + siteName + "')\" "
            + "style=\"cursor:hand;\"></td>"
            + " </tr>"
            + " </table>"
            + "</div>";
}
