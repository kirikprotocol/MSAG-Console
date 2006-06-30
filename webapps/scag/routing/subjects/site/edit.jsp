<%@include file="/WEB-INF/inc/header.jspf"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>

<sm:page>
<jsp:attribute name="title">
    <c:choose>
        <c:when test="${param.add}">subjects.add.title</c:when>
        <c:otherwise>subjects.edit.title</c:otherwise>
    </c:choose>
</jsp:attribute>


<jsp:attribute name="menu">
    <sm-pm:menu>
        <sm-pm:item name="mbSave" value="subjects.edit.item.mbsave.value" title="subjects.edit.item.mbsave.title"/>
        <sm-pm:item name="mbCancel" value="subjects.edit.item.mbcancel.value" title="subjects.edit.item.mbcancel.title" onclick="clickCancel()"/>
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

        function addPathElement(valueElem, pname){
            if(trimStr(valueElem.value).length > 0){
                var tbl = opForm.all.path_table;
                var newRow = tbl.insertRow(tbl.rows.length);
                newRow.className = "row" + ((tbl.rows.length + 1) & 1);
                newRow.id = "srcRow_" + (global_counter++);
                newCell = document.createElement("td");
                newCell.innerHTML = '<img src="content/images/mask.gif">';
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.innerHTML = valueElem.value + '<input type="hidden" + name="' + pname + '" value="' + valueElem.value + '">';
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(opForm.all.path_table, \'' + newRow.id + '\')" style="cursor: hand;">';
                newRow.appendChild(newCell);
                valueElem.value = "";
                valueElem.focus();
                return true;
            }else{
                return false;
            }
        }

        function trimStr(sString) {
            while (sString.substring(0, 1) == ' '){
                sString = sString.substring(1, sString.length);
            }
            while (sString.substring(sString.length - 1, sString.length) == ' '){
                sString = sString.substring(0, sString.length - 1);
            }
            return sString;
        }

      </script>

      <table cellpadding="2">
          <col width="50%">
          <col width="50%">
          <tr>
              <td valign="top">
                  <sm-ep:properties title="subjects.site.info">
                      <br>
                      <sm-ep:txt title="subjects.edit.txt.host" name="host" maxlength="60"/>
                      <sm-ep:txt title="subjects.edit.txt.port" name="port" maxlength="60"/>
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
                      <col width="100%" align="left">
                      <col width="100%" align="left">
                      <tr>
                          <td>path&nbsp;</td>
                          <td><input id="newPath" class="txt" name="pathLinks"></td>
                          <td><img src="content/images/but_add.gif"
                                   onclick="addPathElement(opForm.all.newPath, 'pathLinks')"
                                   style="cursor:hand;"></td>
                      </tr>
                  </table>
                  <hr width="100%" align="left">

                  <c:set var="rowN" value="0"/>
                  <table id="path_table" width="50%" class="properties_list" cellspacing="0"
                         cellpadding="0">
                      <col width="1%">
                      <col width="100%">
                      <col width="100%">
                      <c:forEach items="${bean.pathLinks}" var="i">
                          <c:set var="epathLinks" value="${fn:escapeXml(i)}"/>
                          <tr class="row${rowN%2}" id="pathRow_${epathLinks}">
                              <td><img src="content/images/mask.gif"></td>
                              <td>${epathLinks}<input type="hidden" name="pathLinks" value="${epathLinks}"></td>
                              <td><img src="content/images/but_del.gif"
                                       onClick="removeRow(opForm.all.path_table, 'pathRow_${epathLinks}')"
                                       style="cursor:hand;"></td>
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