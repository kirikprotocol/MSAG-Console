<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="options.index.title">
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbSave" value="options.item.mbsave.value" title="options.item.mbsave.title"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>

    <script type="text/javascript">
        var global_counter = 0;
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
                var strY = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                        + siteUsrPlaceName + " Default USR place\"  onClick=\"removeRow(opForm.all.site_usr_tbl, " + "\'" + newRow.id + '\');\" style=\"cursor:hand;\"/>';
                newCell.innerHTML = strY;
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
                var strY = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                        + abonUsrPlaceName + " Default USR place\"  onClick=\"removeRow(opForm.all.abon_usr_tbl, " + "\'" + newRow.id + '\');\" style=\"cursor:hand;\"/>';
                newCell.innerHTML = strY;
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
                var strY = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                        + sitePlaceName + " Default Address place\"  onClick=\"removeRow(opForm.all.site_address_tbl, " + "\'" + newRow.id + '\');\" style=\"cursor:hand;\"/>';
                newCell.innerHTML = strY;
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
                newCell.innerHTML = getSelectType(addrPlaceName, inner__counter, selectElem, 'addressSelect_' );
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
                var strY = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                        + addrPlaceName + " Global Address place\"  onClick=\"removeRow(opForm.all.abon_address_tbl, " + "\'" + newRow.id + '\');\" style=\"cursor:hand;\"/>';
                newCell.innerHTML = strY;
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
            for (i = 0; i < selectTypeArr.length; i++) {
                if (selVal == selectTypeArr[i]) {
                    strSel += "        <option value=\"" + selectTypeArr[i] + "\" selected=\"true\">" + selectTypeArr[i] + "</option>\n";
                } else {
                    strSel += "        <option value=\"" + selectTypeArr[i] + "\">" + selectTypeArr[i] + "</option>\n";
                }
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

    </script>
    <table cellpadding="0" cellspacing="0" width="100%">
        <tr>
            <td align="left"><div class="page_subtitle_medium">Abonent Options</div></td>
            <td align="left"><div class="page_subtitle_medium">Site Options&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</div></td>
        </tr>
        <tr>
            <td align="left"><hr></td>
            <td align="left"><hr></td>
        </tr>
    </table>
    <br>
    <table cellpadding="0" cellspacing="0" width="100%">
        <tr>
            <td align="left"><div class="page_subtitle">Global Address place:</div></td>
            <td align="left"><div class="page_subtitle">Default Address place:</div></td>
        </tr>
    </table>
    <br>
    <table width="100%" cellpadding="0" cellspacing="0">
        <tr><td>

        <table id="abon_address_tbl" class="properties_list" cellpadding="0" cellspacing="0">
            <c:set var="rowN" value="0"/>
            <c:forEach items="${bean.abonentAddress}" var="pls">
                <tr><td>
                <tr class="row${rowN%2}" id="address_place_${pls.name}_${rowN}">
                    <td width="7%"><select id="addressSelect_${pls.name}_${rowN}" name="addressSelect_${pls.name}_${rowN}" class="txt">
                        <c:forEach items="${bean.optionTypes}" var="i">
                            <c:choose>
                                <c:when test="${pls.type==i}">
                                    <option value="${fn:escapeXml(i)}" selected="true">${fn:escapeXml(i)}</option>
                                </c:when>
                                <c:otherwise>
                                    <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                </c:otherwise>
                            </c:choose>
                        </c:forEach>
                    </select>
                    </td>
                    <td>&nbsp;</td>
                    <%--<td width="43%">Priority:&nbsp;${pls.name}</td>--%>
                    <td>
                        <input type="text" size="50" style="font:bold; color:black;" value="${pls.name}" readonly="true"/>
                        <input type="hidden" name="abonAddrName" value="${pls.name}">
                        <input type="hidden" name="abonAddrType" value="${pls.name}_${rowN}">
                    </td>
                    <%--<td>Priority:&nbsp;${pls.priority}</td>--%>
                    <td width="65%"><img src="content/images/but_del.gif" alt="" onClick="removeRow(opForm.all.abon_address_tbl, 'address_place_${pls.name}_${rowN}');"
                                style="cursor:hand;"></td>
                </tr>
                <c:set var="rowN" value="${rowN+1}"/>
            </c:forEach>
             </td></tr>
        </table>
        <!--<br>-->
        <hr width="50%" align="left"/>
        <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
            <col width="40%" align="left">
            <col width="50%" align="left">
            <col width="0%" align="left">
            <tr valign="top">
                <td width="7%"><select id="typeSelectAddress" name="typeSelectAddress" class="txt">
                    <c:forEach items="${bean.optionTypes}" var="i">
                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                    </c:forEach>
                </select>
                </td>
                <td width="25%">
                    <input id="newAbonAddress" class="txt" size="50" name="newAbonAddress">
                </td>
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Global Address place"
                         onclick="addGlobAddressPlace(opForm.all.newAbonAddress, opForm.all.typeSelectAddress.options[typeSelectAddress.selectedIndex].value)"
                         style="cursor:hand;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
        </td>
        <td>
            <table id="site_address_tbl" class="properties_list" cellpadding="0" cellspacing="0">
                <c:set var="rowN" value="0"/>
                <c:forEach items="${bean.siteAddress}" var="pls">
                    <tr class="row${rowN%3}" id="site_place_${pls.name}_${rowN}">
                        <td width="7%"><select id="siteSelect_${pls.name}_${rowN}" name="siteSelect_${pls.name}_${rowN}" class="txt">
                            <c:forEach items="${bean.optionTypes}" var="i">
                                <c:choose>
                                    <c:when test="${pls.type==i}">
                                        <option value="${fn:escapeXml(i)}" selected="true">${fn:escapeXml(i)}</option>
                                    </c:when>
                                    <c:otherwise>
                                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                    </c:otherwise>
                                </c:choose>
                            </c:forEach>
                        </select>
                        </td>
                        <td>&nbsp;</td>
                            <%--<td width="43%">Priority:&nbsp;${pls.name}</td>--%>
                        <td>
                            <input type="text" size="50" style="font:bold; color:black;" value="${pls.name}" readonly="true"/>
                            <input type="hidden" name="siteAddrName" value="${pls.name}">
                            <input type="hidden" name="siteAddrType" value="${pls.name}_${rowN}">
                        </td>
                            <%--<td>Priority:&nbsp;${pls.priority}</td>--%>
                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow(opForm.all.site_address_tbl, 'site_place_${pls.name}_${rowN}');"
                                             style="cursor:hand;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>

            </table>
            <!--<br>-->
            <hr width="50%" align="left"/>
            <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
                <col width="40%" align="left">
                <col width="50%" align="left">
                <col width="0%" align="left">
                <tr valign="top">
                    <td width="7%"><select id="typeSelectSite" class="txt">
                        <c:forEach items="${bean.optionTypes}" var="i">
                            <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                        </c:forEach>
                    </select>
                    </td>
                    <td width="25%">
                        <input id="new_site_address" class="txt" size="50" name="new_site_address">
                    </td>
                    <td width="68%"><img src="content/images/but_add.gif" alt="Add new Default Address place"
                                         onclick="addDefaultAddressPlace(opForm.all.new_site_address, opForm.all.typeSelectSite.options[typeSelectSite.selectedIndex].value)"
                                         style="cursor:hand;"></td>
                    <td>&nbsp;</td>
                </tr>
            </table>

        </td>
         </tr>
        </table>
    <table width="100%" cellpadding="0" cellspacing="0">
        <tr><td>

        <br>
        <br>
        <br>
        <div class="page_subtitle">Default USR place:</div>
        <br>
            <table id="abon_usr_tbl" class="properties_list" cellpadding="0" cellspacing="0">
                <c:set var="rowN" value="0"/>
                <c:forEach items="${bean.abonentUsr}" var="pls">
                    <tr class="row${rowN%3}" id="abon_usr_place_${pls.name}_${rowN}">
                        <td width="7%"><select id="abonUsrSelect_${pls.name}_${rowN}"
                                               name="abonUsrSelect_${pls.name}_${rowN}" class="txt">
                            <c:forEach items="${bean.optionTypes}" var="i">
                                <c:choose>
                                    <c:when test="${pls.type==i}">
                                        <option value="${fn:escapeXml(i)}" selected="true">${fn:escapeXml(i)}</option>
                                    </c:when>
                                    <c:otherwise>
                                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                    </c:otherwise>
                                </c:choose>
                            </c:forEach>
                        </select>
                        </td>
                        <td>&nbsp;</td>
                            <%--<td width="43%">Priority:&nbsp;${pls.name}</td>--%>
                        <td>
                            <input type="text" size="50" style="font:bold; color:black;" value="${pls.name}"
                                   readonly="true"/>
                            <input type="hidden" name="abonentUsrName" value="${pls.name}">
                            <input type="hidden" name="abonentUsrType" value="${pls.name}_${rowN}">
                        </td>

                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow(opForm.all.abon_usr_tbl, 'abon_usr_place_${pls.name}_${rowN}');"
                                             style="cursor:hand;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
            </table>
        <!--<br>-->
        <hr width="50%" align="left"/>
        <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
            <col width="40%" align="left">
            <col width="50%" align="left">
            <col width="0%" align="left">
            <tr valign="top">
                <td width="7%"><select id="typeAbonSelectUsr" name="typeAbonSelectUsr" class="txt">
                    <c:forEach items="${bean.optionTypes}" var="i">
                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                    </c:forEach>
                </select>
                </td>
                <td width="25%">
                    <input id="new_abon_usr" class="txt" size="50" name="new_abon_usr">
                </td>
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Abonent USR"
                         onclick="addAbonDefaultUSRplace(opForm.all.new_abon_usr, opForm.all.typeAbonSelectUsr.options[typeAbonSelectUsr.selectedIndex].value)"
                         style="cursor:hand;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
    </td>
        <td>

        <br>
        <br>
        <br>
        <div class="page_subtitle">Default USR place:</div>
        <br>
            <table id="site_usr_tbl" class="properties_list" cellpadding="0" cellspacing="0">
                <c:set var="rowN" value="0"/>
                <c:forEach items="${bean.siteUsr}" var="pls">
                    <tr class="row${rowN%3}" id="site_usr_place_${pls.name}_${rowN}">
                        <td width="7%"><select id="siteUsrSelect_${pls.name}_${rowN}"
                                               name="siteUsrSelect_${pls.name}_${rowN}" class="txt">
                            <c:forEach items="${bean.optionTypes}" var="i">
                                <c:choose>
                                    <c:when test="${pls.type==i}">
                                        <option value="${fn:escapeXml(i)}" selected="true">${fn:escapeXml(i)}</option>
                                    </c:when>
                                    <c:otherwise>
                                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                    </c:otherwise>
                                </c:choose>
                            </c:forEach>
                        </select>
                        </td>
                        <td>&nbsp;</td>
                            <%--<td width="43%">Priority:&nbsp;${pls.name}</td>--%>
                        <td>
                            <input type="text" size="50" style="font:bold; color:black;" value="${pls.name}"
                                   readonly="true"/>
                            <input type="hidden" name="siteUsrName" value="${pls.name}">
                            <input type="hidden" name="siteUsrType" value="${pls.name}_${rowN}">
                        </td>
                            <%--<td>Priority:&nbsp;${pls.priority}</td>--%>
                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow(opForm.all.site_usr_tbl, 'site_usr_place_${pls.name}_${rowN}');"
                                             style="cursor:hand;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
            </table>
        <!--<br>-->
        <hr width="50%" align="left"/>
        <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
            <col width="40%" align="left">
            <col width="50%" align="left">
            <col width="0%" align="left">
            <tr valign="top">
                <td width="7%"><select id="typeSelectUsrSite" class="txt">
                    <c:forEach items="${bean.optionTypes}" var="i">
                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                    </c:forEach>
                </select>
                </td>
                <td width="25%">
                    <input id="new_site_usr" class="txt" size="50" name="new_site_usr">
                </td>
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Site"
                         onclick="addSiteDefaultUSRplace(opForm.all.new_site_usr, opForm.all.typeSelectUsrSite.options[typeSelectUsrSite.selectedIndex].value)"
                         style="cursor:hand;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
        </td>
    </tr>
    </table>


    </jsp:body>
</sm:page>