<%@ include file="/WEB-INF/inc/header.jspf" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<sm:page onLoad="commonOnLoad()">
<jsp:attribute name="title">
    <c:choose>
        <c:when test="${param.add}">routes.add.title</c:when>
        <c:otherwise>routes.edit.title</c:otherwise>
    </c:choose>
</jsp:attribute>

<jsp:attribute name="menu">                         
    <sm-pm:menu>
        <sm-pm:item name="mbSave" value="routes.edit.item.mbsave.value" title="routes.edit.item.mbsave.title"/>
        <sm-pm:item name="mbCancel" value="routes.edit.item.mbcancel.value" title="routes.edit.item.mbcancel.title" onclick="clickCancel()"/>
        <sm-pm:space/>
    </sm-pm:menu>
</jsp:attribute>

<jsp:body>
<script type="text/javascript">
    var global_counter = 0;

    function commonOnLoad() {
        checkAbonentSubj();
        checkSiteSubj();
    }

    function removeRow(tbl, rowId) {
        var rowElem = tbl.rows(rowId);
        tbl.deleteRow(rowElem.rowIndex);
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

    function removeSiteSubj(rowId){
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
            
    function checkAbonentSubj(){
        var selectElem = opForm.all.abonSubjSelect;
        if (selectElem.options.length < 1) {
            selectElem.disabled = true;
        }
    }

    function checkSiteSubj(){
        var selectElem = opForm.all.siteSubjSelect;
        if(selectElem.options.length < 1){
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
            newRow.id = "abonRow_" + (global_counter++);
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


    function addSiteSubj(){
        var selectElem = opForm.all.siteSubjSelect;
        if(selectElem.options.length > 0){
            var subjValue = selectElem.options[selectElem.selectedIndex].value;
            var tbl = opForm.all.sites_table;
            var newRow = tbl.insertRow(tbl.rows.length);
            newRow.className = "row" + ((tbl.rows.length + 1) & 1);
            newRow.id = "siteSubRow_" + (global_counter++);
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
            newRow.id = "abonRow_" + (global_counter++);
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
            newRow.id = "pathRow_" + (global_counter++);
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

    function trimStr(sString) {
        while (sString.substring(0, 1) == ' ') {
            sString = sString.substring(1, sString.length);
        }
        while (sString.substring(sString.length - 1, sString.length) == ' ') {
            sString = sString.substring(0, sString.length - 1);
        }
        return sString;
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

    function addSite(valueElem, portElem) {
        if (!trimStr(portElem.value).length > 0)
            portElem.value = '80';
        if ((trimStr(valueElem.value).length > 0) && validateField(portElem)) {

            var siteName = valueElem.value.substr(0, valueElem.value.indexOf('.'));

            var tbl = opForm.all.div_site_table;
            var newRow = tbl.insertRow(tbl.rows.length);
            newCell = newRow.insertCell();
            newCell.innerHTML = addSiteSectionBody(siteName, valueElem.value, portElem.value);
            newRow.appendChild(newCell);
            valueElem.value = "";
            portElem.value = '80';
            valueElem.focus();
            return true;
        } else
            return false;
    }

    function addSiteSectionBody(siteName, siteFullName, sitePort) {
        var sectionHeader_ = "sectionHeader_" + siteName;
        return ""
                + "<div class=\"collapsing_tree_opened\" id=\"" + sectionHeader_ + "\" onclick=\"collasping_tree_showhide_section('" + siteName + "')\">"
                + " <table id=\"sitesp_table_" + siteName + "\" cellpadding=\"0\" cellspacing=\"0\" class=\"properties_list\">"
                + " <col width=\"1%\">"
                + " <col width=\"99%\">"
                + " <tr>"
                + " <td width=\"100%\">" + siteFullName + "</td>"
                + " <td align=\"left\" nowrap=\"true\"><input type=\"hidden\" name=\"sitesHost\" value=\"" + siteFullName + "\">"
                + " <input type=\"hidden\" name=\"sitesPort\" value=\"" + siteFullName + "_" + sitePort + "\"></td>"
                + " <td><img src=\"content/images/but_del.gif\" onClick=\"removeSection('" + siteName + "')\"></td>"
                + " </tr>"
                + " <tr>"
                + " <td nowrap=\"true\">Add new path&nbsp;</td>"
                + " <td align=\"right\"><input id=\"" + siteName + "_newPath\" class=\"txt\" name=\"" + siteName + "_newPath\"></td>"
                + " <td><img src=\"content/images/but_add.gif\" alt=\"Add new path\" "
                + "onclick=\"addPath(opForm.all." + siteName + "_newPath, '" + siteFullName + "', opForm.all.sitesp_table_" + siteName + ", '" + siteName + "')\" "
                + "style=\"cursor:hand;\"></td>"
                + " </tr>"
                + " </table>"
                + "</div>";
    }

</script>

<table cellpadding="5">
    <col width="50%">
    <col width="50%">
    <tr>
        <td valign="top">
            <sm-ep:properties title="routes.edit.properties.route_info" noColoredLines="false">
                <sm-ep:txt title="routes.edit.txt.id" name="id" maxlength="60" readonly="${!bean.add}" validation="nonEmpty"/>
            </sm-ep:properties>
        </td>
        <td valign="top">
            <sm-ep:properties title="routes.edit.properties.route_options" noColoredLines="false" noHeaders="false">
                <sm-ep:check title="routes.edit.check.enabled" name="enabled"/>
                <sm-pm:space/>
            </sm-ep:properties>
        </td>
    </tr>
    <tr><td colspan="2"><hr></td></tr>
</table>
<table cellpadding="12">
    <col width="42%">
    <col width="16%">
    <col width="42%">
    <tr>
        <td valign="top" width="50%">
            <div class="page_subtitle">Abonents</div>
            <table cellpadding="0" cellspacing="0">
                <col width="50%" align="left">
                <col width="50%" align="right">
                <col width="0%" align="left">
                <tr valign="middle">
                    <td>Subject</td>
                    <td><select id="abonSubjSelect" class="txt">
                        <c:forEach items="${bean.allUncheckedAbonSubjects}" var="i">
                            <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                        </c:forEach>
                    </select>
                    </td>
                    <td><img src="content/images/but_add.gif" onclick="addAbonentSubj()" style="cursor:hend;"></td>
                </tr>
                <tr>
                    <td>Address</td>
                    <td><input id="newAbonMask" class="txt" name="newAbonMask" validation="routeMask"
                               onkeyup="resetValidation(this)"></td>
                    <td><img src="content/images/but_add.gif" onclick="addAbonentMask(opForm.all.newAbonMask)"
                             alt="Add Http Subject" style="cursor: hand;"></td>
                </tr>
            </table>
            <hr>
            <c:set var="rowN" value="0"/>
            <table id="abonents_table" class="properties_list" cellpadding="0" cellspacing="0">
                <col width="1%">
                <col width="100%">
                <c:forEach items="${bean.abonSubj}" var="i">
                        <c:set var="esubj" value="${fn:escapeXml(i)}"/>
                <tr class="row${rowN%3}" id="subjRow_${esubj}">
                    <td><img src="content/images/subject.gif" alt=""></td>
                    <td>${esubj}<input id="subjAbon" type="hidden" name="abonSubj" value="${esubj}"></td>
                    <td><img src="content/images/but_del.gif" alt="" onclick="removeAbonetSubj('subRow_${esubj}');"
                             style="cursor: hand;"></td>
                </tr>
                <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
                <c:forEach items="${bean.abonAddress}" var="i">
                    <c:set var="emask" value="${fn:escapeXml(i)}"/>
                    <tr class="row${rowN%2}" id="maskRow_${emask}">
                        <td><img src="content/images/mask.gif"></td>
                        <td>${emask}<input type="hidden" name="abonAddress" value="${emask}"></td>
                        <td><img src="content/images/but_del.gif"
                                 onClick="removeRow(opForm.all.abonents_table, 'maskRow_${emask}')"
                                 style="cursor: hand;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
            </table>
        </td>
        <td valign="top" width="50%">
            <div class="page_subtitle">Sites</div>
            <table cellpadding="0" cellspacing="0">
                 <col width="50%" align="left">
                 <col width="50%" align="right">
                 <col width="0%" align="left">
                <tr>
                    <td>Site Subject</td>
                    <td>
                        <select id="siteSubjSelect" class="txt">
                            <c:forEach items="${bean.allUncheckedSiteSubject}" var="i">
                                <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                            </c:forEach>
                        </select>
                    </td>
                    <td><img src="content/images/but_add.gif" alt="HTTP Site List" onclick="addSiteSubj()" style="cursor:hand;"></td>
                </tr>
                <tr>
                    <td>Site/Port</td>
                    <td nowrap="true"><input id="newSite" class="midtxt" name="newSite"><b>:</b><input id="newPort" class="mintxt" name="newPort" value="80" validation="port" onkeyup="resetValidation(this)"></td>
                    <td><img src="content/images/but_add.gif" alt="Add new Site" onclick="addSite(opForm.all.newSite,opForm.all.newPort)"
                        style="cursor: hand;"></td>
                </tr>
            </table>
            <hr>
            <c:set var="rowN" value="0"/>
            <table id="sites_table" class="properties_list" cellpadding="0" cellspacing="0">
                <col width="1%">
                <col width="99%">
                <c:forEach items="${bean.siteSubj}" var="i">
                    <c:set var="ssubj" value="${fn:escapeXml(i)}"/>
                    <tr class="row${rowN%3}" id="siteSubRow_${ssubj}">
                        <td><img src="content/images/subject.gif" alt=""><img src="content/images/subject.gif" alt=""></td>
                        <td>${ssubj}<input id="subSite" type="hidden" name="siteSubj" value="${ssubj}"></td>
                        <td><img src="content/images/but_del.gif" alt="" onclick="removeSiteSubj('siteSubRow_${ssubj}');"
                                style="cursor:hand;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
          </table>
            <sm-et:section title="Sites" name="allsites">
                <table id="div_site_table" cellpadding="0" cellspacing="0" class="properties_list">
                    <tr><td>
                        <c:forEach items="${bean.sites}" var="i">
                            <c:set var="esite" value="${fn:escapeXml(i.host)}"/>
                            <c:set var="eport" value="${fn:escapeXml(i.port)}"/>
                            <c:set var="esite_sub" value="${fn:substringBefore(esite,'.')}"/>
                            <c:set var="sectHeader" value="sectionHeader_${esite_sub}"/>
                            <c:set var="esite_table" value="sitesp_table_${esite_sub}"/>

                            <div class="collapsing_tree_opened" id="${sectHeader}"
                                 onclick="collasping_tree_showhide_section('${esite_sub}')">

                                <table id="${esite_table}" cellpadding="0" cellspacing="0" class="properties_list">
                                    <col width="1%">
                                    <col width="99%">
                                    <tr>
                                        <td width="100%">${fn:escapeXml(i.host)}</td>
                                        <td align="left" nowrap="true"><input type="hidden" name="sitesHost"
                                                                              value="${esite}">
                                            <input type="hidden" name="sitesPort" value="${esite}_${eport}"></td>
                                        <td><img src="content/images/but_del.gif"
                                                 onClick="removeSection('${esite_sub}')"
                                                 style="cursor:hand;"></td>
                                    </tr>

                                    <tr>
                                        <td nowrap="true">Add new path&nbsp;</td>
                                        <td align="right"><input id="${esite_sub}_newPath" class="txt"
                                                                 name="${esite_sub}_newPath"></td>
                                        <td><img src="content/images/but_add.gif" alt="Add new path"
                                                 onclick="addPath(opForm.all.${esite_sub}_newPath, '${esite}', opForm.all.sitesp_table_${esite_sub}, '${esite_sub}')"
                                                 style="cursor:hand;"></td>
                                    </tr>
                                    <c:forEach items="${i.pathLinks}" var="j">
                                        <c:set var="epath" value="${fn:escapeXml(j)}"/>
                                        <tr class="row${rowN%3}" id="pathRow_${esite_sub}_${epath}">
                                            <td></td>
                                            <td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;${epath}<input type="hidden"
                                                                                             name="pathLinks"
                                                                                             value="${esite}_${epath}">
                                            </td>
                                            <td><img src="content/images/but_del.gif"
                                                     onClick="removeRow(opForm.all.sitesp_table_${esite_sub}, 'pathRow_${esite_sub}_${epath}')"
                                                     style="cursor:hand;"></td>
                                        </tr>
                                    </c:forEach>
                                </table>
                            </div>
                        </c:forEach>
                    </td></tr>
                </table>
            </sm-et:section>
        </td>
    </tr>
</table>
</jsp:body>
</sm:page>