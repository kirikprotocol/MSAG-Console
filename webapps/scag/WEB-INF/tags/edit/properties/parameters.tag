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
    <c:set var="values" value="${fn:split(values, ';')}"/>
    <script src="content/scripts/counters.js" type="text/javascript"></script>
    <table id="temp_param_tbl" class="properties_list" cellpadding="0" cellspacing="0">
        <thead>
            <tr>
                <td align="left">
                    <c:if test="${!empty first_field_name}"><fmt:message>${fn:escapeXml(first_field_name)}</fmt:message></c:if>
                </td>
                <td align="left" colspan="2">
                    <c:if test="${!empty second_field_name}"><fmt:message>${fn:escapeXml(second_field_name)}</fmt:message></c:if>
                </td>               
            </tr>
        </thead>
        <c:set var="rowN" value="0"/>
        <c:forEach items="${bean.parameters}" var="parameter">
            <tr class="row${rowN%2}" id="param_${parameter.name}_${rowN}">
                <td>
                    <input id="par_name_${parameter.name}_${rowN}" name="parameter.${rowN}.name" type="text" size="45" style="color:black;" value="${parameter.name}" readonly="true"/>
                </td>
                <td>
                    <input id="par_value_${parameter.name}_${rowN}" name="parameter.${rowN}.value" type="text" size="45" style="color:black;" value="${parameter.value}" readonly="true"/>                    
                </td>
                <td>
                    <img src="content/images/but_del.gif" alt="Add new parameter"
                         style="cursor:pointer;"
                         onClick="removeRow('temp_param_tbl', 'param_${parameter.name}_${rowN}');"/>
                </td>                
            </tr>
            <c:set var="rowN" value="${rowN+1}"/>
        </c:forEach>
        <script type="text/javascript">
            param_counter = ${rowN};
        </script>
    </table>
    <table class="properties_list" cellpadding="0" cellspacing="0">
        <tr class="row${rowN%2}" >
            <td><input id="pName" type="text" size="45" style="color:black;" value="" /></td>
            <td><input id="pValue" type="text" size="45" style="color:black;" value="" /></td>
            <td><img src="content/images/but_add.gif" alt="Add new parameter"
                style="cursor:pointer;" onclick="addParameter('pName','pValue')"/></td>
        </tr>
    </table>
</sm-ep:property>