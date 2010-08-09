<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<%@taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt" %>
<%@tag body-content="empty" %>
<%@attribute name="title"      required="true"%>
<%@attribute name="first_field_name" required="true"%>
<%@attribute name="second_field_name" required="true"%>
<%@attribute name="values" required="true" %>
<sm-ep:property title="${title}">
    <script src="content/scripts/counters.js" type="text/javascript"></script>
    <table id="parameter.tbl" class="properties_list" cellpadding="3" cellspacing="1">
        <tr>
            <td align="left">
                <c:if test="${!empty first_field_name}">
                    <fmt:message>${fn:escapeXml(first_field_name)}</fmt:message>
                </c:if>
            </td>
            <td align="left" colspan="2">
                <c:if test="${!empty second_field_name}">
                    <fmt:message>${fn:escapeXml(second_field_name)}</fmt:message>
                </c:if>
            </td>
        </tr>        
        <c:set var="rowN" value="0"/>
        <tr class="row${rowN%2}" >
            <td>
                <input id="pName"
                       type="text"
                       size="45"
                       style="color:black;"
                       value=""
                       onkeyup="resetValidation(this)"/>
            </td>
            <td>
                <input id="pValue"
                       type="text"
                       size="45"
                       style="color:black;"
                       value=""
                       onkeyup="resetValidation(this)"/>
            </td>
            <td width="100%"><img src="content/images/but_add.gif" alt="Add new parameter"
                style="cursor:pointer;"
                onclick="addRow('pName','pValue','parameter','name','value')"/></td>
        </tr>        
        <c:if test="${fn:length(values)>0}">
            <c:set var="va" value="${fn:split(values, ';')}"/>            
            <c:forEach items="${va}" var="vp">
                <c:set var="sI" value="${fn:indexOf(vp, ',')}"/>
                <c:set var="pName" value="${fn:substring(vp, 0, sI)}"/>
                <c:set var="pValue" value="${fn:substring(vp, sI+1, fn:length(vp))}"/>
                <tr class="row${rowN%2}" id="parameter.${pName}.${rowN}">
                    <td>
                        <input id="parameter.name.${pName}.${rowN}"
                               name="parameter.${rowN}.name"
                               type="text"
                               size="45"
                               style="color:black;"
                               value="${pName}"
                               readonly="true"/>
                    </td>
                    <td>
                        <input id="parameter.value.${pName}.${rowN}"
                               name="parameter.${rowN}.value"
                               type="text"
                               size="45"
                               style="color:black;"
                               value="${pValue}"
                               readonly="true"/>
                    </td>
                    <td width="100%">
                        <img src="content/images/but_del.gif" alt="Add new parameter"
                             style="cursor:pointer;"
                             onClick="removeRow('parameter.tbl', 'parameter.${pName}.${rowN}');"/>
                    </td>
                </tr>
                <c:set var="rowN" value="${rowN+1}"/>
            </c:forEach>
        </c:if>
        <script type="text/javascript">
            param_counter = ${rowN};
        </script>
    </table>

</sm-ep:property>