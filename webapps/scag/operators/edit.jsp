<%@ include file="/WEB-INF/inc/header.jspf" %>
<%@
        taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<sm:page>
<jsp:attribute name="title">
    <c:choose>
        <c:when test="${param.add}">operators.add.title</c:when>
        <c:otherwise>operators.edit.title</c:otherwise>
    </c:choose>
</jsp:attribute>

<jsp:attribute name="menu">
    <sm-pm:menu>
        <sm-pm:item name="mbSave" value="operators.edit.item.mbsave.value" title="operators.edit.item.mbsave.title"/>
        <sm-pm:item name="mbCancel" value="operators.edit.item.mbcancel.value" title="operators.edit.item.mbcancel.title" onclick="clickCancel()"/>
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

        function addMask(valueName) {
            var valueElem = getElementByIdUni( valueName );
            if (validateField(valueElem)) {
                var tbl = getElementByIdUni("sources_table");
                var newRow = tbl.insertRow(tbl.rows.length);
                newRow.className = "row" + ((tbl.rows.length + 1) & 1);
                newRow.id = "srcRow_" + (global_counter++);
                newCell = document.createElement("td");
                newCell.innerHTML = '<img src="content/images/mask.gif">';
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.innerHTML = valueElem.value + '<input type=hidden name=srcMasks value="' + valueElem.value + '">';
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(\'sources_table\', \'' + newRow.id + '\')" style="cursor: pointer;">';
                newRow.appendChild(newCell);
                valueElem.value = "";
                valueElem.focus();
                return true;
            } else
                return false;
        }

    </script>
    <table cellpadding="2">
        <col width="50%">
        <col width="50%">
        <tr>
            <td valign="top">
                <sm-ep:properties title="operators.edit.properties">
                    <c:if test="${!bean.add}"><input type="hidden" name="id" id="id"
                                                     value="${fn:escapeXml(bean.id)}"></c:if>
                    <sm-ep:txt title="operators.edit.txt.name" name="name" validation="nonEmpty"/>
                    <sm-ep:txtBox title="operators.edit.txtbox.description" cols="0" rows="0" name="description"/>
                </sm-ep:properties>
            </td>
            <td valign="top">&nbsp;</td>
            <tr><td colspan="2">&nbsp;</td></tr>
    </table>

    <table cellpadding="2">
        <col width="50%">
        <col width="50%">
        <tr>
            <td valign="top">
                <table cellspacing="0" width="50%" cellpadding="0">
                    <col width="10%" align="left">
                    <col width="100%" align="right">
                    <col width="100%" align="left">
                    <tr>
                        <td><fmt:message>operators.edit.label.mask</fmt:message></td>
                        <td><input id="newSrcMask" class="txt" name="srcMasks" validation="routeMask"
                                   onkeyup="resetValidation(this)"></td>
<%--                        <td><img src="content/images/but_add.gif" onclick="addMask(opForm.all.newSrcMask)"--%>
                        <td><img src="content/images/but_add.gif" onclick="addMask('newSrcMask')"
                                 style="cursor:pointer;"></td>
                    </tr>
                </table>
                <hr width="100%" align="left">

                <c:set var="rowN" value="0"/>
                <table id="sources_table" width="50%" class="properties_list" cellspacing="0" cellpadding="0">
                    <col width="1%">
                    <col width="100%">
                    <col width="100%">
                    <c:forEach items="${bean.srcMasks}" var="i">
                        <c:set var="emask" value="${fn:escapeXml(i)}"/>
                        <tr class="row${rowN%2}" id="maskRow_${emask}">
                            <td><img src="content/images/mask.gif"></td>
                            <td>${emask}<input type="hidden" name="srcMasks" value="${emask}"></td>
                            <td><img src="content/images/but_del.gif"
                                     onClick="removeRow('sources_table', 'maskRow_${emask}')"
                                     style="cursor: pointer;"></td>
                        </tr>
                        <c:set var="rowN" value="${rowN+1}"/>
                    </c:forEach>
                </table>
            </td>
            <td valign="top">&nbsp;</td>
            <tr><td colspan="2">&nbsp;</td></tr>
    </table>

</jsp:body>
</sm:page>