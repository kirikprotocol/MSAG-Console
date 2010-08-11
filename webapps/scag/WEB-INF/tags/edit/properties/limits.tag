<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<%@taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt" %>
<%@tag body-content="empty" %>
<%@attribute name="title"      required="true"%>
<%@attribute name="first_field_name" required="true"%>
<%@attribute name="second_field_name" required="true"%>
<%@attribute name="values" required="true" %>
<%@attribute name="levels" required="true" %>
<sm-ep:property title="${title}">
    <table id="limit.tbl" class="properties_list" cellspacing="3" cellpadding="1">
        <c:set var="rowN" value="0"/>
        <tr class="row${rowN%2}">
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
        <tr class="row${(rowN+1)%2}">
            <td><input id="lPercent"
                       class="txt"
                       name="limitPercent"                       
                       onkeyup="resetValidation(this)">
            </td>
            <c:set var="lla" value="${fn:split(levels, ',')}"/>
            <td><select id="lLevel" name="limitLevel">
                    <c:forEach items="${lla}" var="ll">
                        <option value="${ll}">${ll}</option>
                    </c:forEach>
                </select>
            </td>
            <td width="100%">
                <img src="content/images/but_add.gif"
                     onclick="addRow('lPercent', 'lLevel','limit','percent','level')"
                     style="cursor:pointer;" alt="Add limit">
            </td>
        </tr>
        <c:if test="${fn:length(values)>0}">
        <c:set var="la" value="${fn:split(values, ';')}"/>
        <c:forEach items="${la}" var="vl"> <!--  vl -  limits values   -->
            <c:set var="lOp" value="${fn:substring(vl, fn:length(vl)-2, fn:length(vl))}"/>
            <c:choose>
                <c:when test="${lOp == 'GE'}">
                    <c:set var="lOp" value=">"/>
                </c:when>
                <c:when test="${lOp == 'LE'}">
                    <c:set var="lOp" value="<"/>
                </c:when>
            </c:choose>            
            <c:set var="vl" value="${fn:substring(vl, 0, fn:length(vl) - 3)}"/>
            <c:set var="sI" value="${fn:indexOf(vl, ',')}"/><!--sI - separator index-->
            <c:set var="lPercent" value="${fn:substring(vl, 0, sI)}"/>
            <c:set var="lLevel" value="${fn:substring(vl, sI+1, fn:length(vl))}"/>
            <tr class="row${rowN%2}" id="limit_${lPercent}_${rowN}">
                <td>
                    <input id="limit_percent_${lOp}${lPercent}_${rowN}"
                           name="limit.${rowN}.percent"
                           type="text"
                           size="45"
                           style="color:black;"
                           value="${lOp}${lPercent}"
                           readonly="true"/>
                </td>
                <td>
                    <input id="limit_level_${lOp}${lPercent}_${rowN}"
                           name="limit.${rowN}.level"
                           type="text"
                           size="45"
                           style="color:black;"
                           value="${lLevel}"
                           readonly="true"/>
                </td>
                <td width="100%">
                    <img src="content/images/but_del.gif" alt="Add new parameter"
                         style="cursor:pointer;"
                         onClick="removeRow('limit.tbl', 'limit_${lPercent}_${rowN}');"/>
                </td>
            </tr>
            <c:set var="rowN" value="${rowN+1}"/>
            </c:forEach>
        </c:if>
        <script type="text/javascript">
                limit_counter = ${rowN};
        </script>
    </table>

</sm-ep:property>
        