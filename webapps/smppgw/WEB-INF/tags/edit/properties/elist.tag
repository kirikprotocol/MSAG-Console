<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="sm" tagdir="/WEB-INF/tags"%><%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %><%@
 tag body-content="empty" %><%@
 attribute name="title"      required="true"%><%@
 attribute name="name"       required="true"%><%@
 attribute name="readonly"   required="false"%><%@
 attribute name="type"       required="false"%><%@
 attribute name="validation" required="false"%>
<c:set var="ename" value="${fn:escapeXml(name)}"/>
<c:set var="input_type" value="${empty type ? 'text' : type}"/>
<sm-ep:property title="${title}">
<table id="table_${ename}" width="100%" cellspacing="0" cellpadding="0">
<col width="1%">
<col width="99%">
  <c:set var="table_counter" value="0"/>
  <c:forEach var="i" items="${bean[name]}">
    <tr id="table_${ename}_${table_counter}">
      <td><input class=txt type="${input_type}" name="${ename}" value="${fn:escapeXml(i)}"
                <c:if test="${readonly}">readonly</c:if>
                <c:if test="${!empty validation}">validation="${validation}"  onkeyup="resetValidation(this)"</c:if>
          ></td>
      <td><sm:button type="del" title="Remove this mask" onClick="return remove_table_row_${ename}('table_${ename}_${table_counter}');"/></td>
    </tr>
    <c:set var="table_counter" value="${table_counter+1}"/>
  </c:forEach>

  <tr>
    <td><input id="table_${ename}_add" class=txt type="${input_type}" name="${fn:escapeXml(name)}" value="${fn:escapeXml(i)}"
              <c:if test="${readonly}">readonly</c:if>
              <c:if test="${!empty validation}">validation="${validation}"  onkeyup="resetValidation(this)"</c:if>
        ></td>
    <td><sm:button type="add" title="Add this mask" onClick="return add_table_row_${name}();"/></td>
  </tr>
</table>
<script>
var ${name}_counter = ${table_counter};

function remove_table_row_${name}(rowId) {
  var tbl = opForm.all.table_${name};
  var rowElem = tbl.rows(rowId);
  tbl.deleteRow(rowElem.rowIndex);
  return false;
}
function add_table_row_${name}()
{
  var eval = opForm.all['table_${ename}_add'];
  var val = eval.value;
  eval.value = "";
  eval.focus();

  var tbl = opForm.all.table_${name};
  var newRow = tbl.insertRow(tbl.rows.length-1);
  //newRow.className = "row" + (tbl.rows.length & 1);
  newRow.id = "table_${ename}_" + (${name}_counter);
  var newCell = document.createElement("td");
  newCell.innerHTML = '<input class=txt type="${input_type}" name="${ename}" value="' + val + '" '
                      + '<c:if test="${readonly}">readonly</c:if> '
                      + '<c:if test="${!empty validation}">validation="${validation}"  onkeyup="resetValidation(this)"</c:if> >';
  newRow.appendChild(newCell);
  newCell = document.createElement("td");
  newCell.innerHTML = '<sm:button type="del" title="Remove this mask" onClick="return remove_table_row_${ename}(\\\'table_${ename}_' + ${name}_counter + '\\\');"/>';
  newRow.appendChild(newCell);
  ${name}_counter++;
  return false;
}
</script>
</sm-ep:property>