<%@include file="/WEB-INF/inc/header.jspf"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<sm:page>
    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">statistics.counters.ca.add.title</c:when>
            <c:otherwise>statistics.counters.ca.edit.title</c:otherwise>
        </c:choose>
    </jsp:attribute>

    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbSave"   value="statistics.counters.ca.edit.item.mbsave.value"
                                        title="statistics.counters.ca.edit.item.mbsave.title"/>
            <sm-pm:item name="mbCancel" value="statistics.counters.ca.edit.item.mbcancel.value"
                                        title="statistics.counters.ca.edit.item.mbcancel.title"
                                        onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>

    <jsp:body>
        <script type="text/javascript">

            var global_counter = 0;

            function removeRow(tblName, rowId) {
                var tbl = getElementByIdUni(tblName);
                var rowElem = tbl.rows[rowId];
                tbl.deleteRow(rowElem.rowIndex);
            }

            function addRow(procentName, levelName) {
                var procentElem = getElementByIdUni(procentName);
                var levelElem = getElementByIdUni(levelName);
                if (!validateField(procentElem) || !validateField(levelElem)) return false;
                else {
                    var tbl = getElementByIdUni("limits_table");
                    var newRow = tbl.insertRow(tbl.rows.length);
                    newRow.className = "row" + ((tbl.rows.length + 1) & 1);
                    newRow.id = "limitRow_" + (global_counter++);
                    var newCell = document.createElement("td");
                    newCell.innerHTML = '<input name="limitProcents" class="txt" value="' + procentElem.value + '" readonly="true">';
                    newRow.appendChild(newCell);
                    newCell = document.createElement("td");
                    newCell.innerHTML = '<input name="limitLevels" class="txt" value="' + levelElem.value + '" readonly="true">';
                    newRow.appendChild(newCell);
                    newCell = document.createElement("td");
                    newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(\'limits_table\', \'' + newRow.id + '\')" style="cursor:pointer;" alt="Delete limit">';
                    newRow.appendChild(newCell);
                    procentElem.value = ""; levelElem.value = "";
                    procentElem.focus();
                    return true;
                }
            }
        </script>

        <sm-ep:properties title="statistics.counters.ca.edit.properties">
            <sm-ep:txt title="statistics.counters.ca.edit.txt.id"   name="id" validation="nonEmpty" readonly="${!bean.add}"/>
            <sm-ep:txt title="statistics.counters.ca.edit.txt.min"  name="limitsMin" maxlength="5" validation="unsignedOrEmpty"/>
            <sm-ep:txt title="statistics.counters.ca.edit.txt.max"  name="limitsMax" maxlength="5" validation="unsignedOrEmpty"/>

            <c:forEach items="${bean.parameters}" var="parameter">
                <c:set var="values" value="${values}${parameter.name},${parameter.value};"/>
            </c:forEach>
            <c:set var="values" value="${fn:substring(values, 0, fn:length(values)-1)}"/>
            <sm-ep:parameters title="tag.parameters" values="${values}"
                             first_field_name="tag.parameter.name" second_field_name="tag.parameter.value"/>
        </sm-ep:properties>

        <fmt:message>statistics.counters.ca.label.limits</fmt:message><br/>
        <table cellspacing="0" cellpadding="0">
            <col width="25%" align="left">
            <col width="50%" align="right">
            <col width="25%" align="left">
            <tr>
                <td><fmt:message>statistics.counters.ca.label.condition</fmt:message></td>
                <td><fmt:message>statistics.counters.ca.label.action</fmt:message></td>
                <td>&nbsp;</td>
            </tr>
            <tr>
                <td valign="top" colspan="3">

                    <c:set var="rowN" value="0"/>
                    <table id="limits_table" class="properties_list" cellspacing="0" cellpadding="0">
                        <col width="25%" align="left">
                        <col width="50%" align="right">
                        <col width="25%" align="left">
                        <c:forEach items="${bean.limits}" var="i">
                            <c:set var="epercent" value="${fn:escapeXml(i.percent)}"/>
                            <c:set var="elevel" value="${fn:escapeXml(i.level)}"/>
                            <tr class="row${rowN%2}" id="limitRow_${epercent}">
                                <td><input name="limitProcents" class="txt" value="${epercent}" readonly="true"></td>
                                <td><input name="limitLevels" class="txt" value="${elevel}" readonly="true"></td>
                                <td><img src="content/images/but_del.gif"
                                         onClick="removeRow('limits_table', 'limitRow_${rowN}')"
                                         style="cursor:pointer;" alt="Delete limit"></td>
                            </tr>
                            <c:set var="rowN" value="${rowN+1}"/>
                        </c:forEach>
                    </table>

                    <hr width="100%" align="left">

                    <table cellspacing="0" cellpadding="0">
                        <col width="25%" align="left">
                        <col width="50%" align="right">
                        <col width="25%" align="left">
                        <tr>
                            <td><input id="newLimitProcent" class="txt" name="limitProcents" validation="unsigned"
                                       onkeyup="resetValidation(this)"></td>
                            <!-- TODO: levels from bean by list -->
                            <td><input id="newLimitLevel" class="txt" name="limitLevels"></td>
                            <td><img src="content/images/but_add.gif" onclick="addRow('newLimitProcent', 'newLimitLevel')"
                                     style="cursor:pointer;" alt="Add limit"></td>
                        </tr>
                    </table>

                </td>
            </tr>
        </table>

    </jsp:body>
</sm:page>