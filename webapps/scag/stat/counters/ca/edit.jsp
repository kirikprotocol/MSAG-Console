<%@ page import="ru.sibinco.scag.backend.stat.counters.LevelType" %>
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
        <sm-ep:properties title="statistics.counters.ca.edit.properties">
            <sm-ep:txt title="statistics.counters.ca.edit.txt.id"   name="id" validation="nonEmpty" readonly="${!bean.add}"/>
            <sm-ep:txt title="statistics.counters.ca.edit.txt.min"  name="limitsMin" maxlength="5" validation="unsignedOrEmpty"/>
            <sm-ep:txt title="statistics.counters.ca.edit.txt.max"  name="limitsMax" maxlength="5" validation="unsignedOrEmpty"/>
           

            <script src="content/scripts/counters.jsp" type="text/javascript" charset="UTF-8"></script>            
            <!-- Get limits -->
            <c:set var="lValues" value=""/>
            <c:forEach items="${bean.limits}" var="l">
                <c:set var="lValues" value="${lValues}${l.percent},${l.level},${l.op};"/>
            </c:forEach>
            <c:if test="${fn:length(lValues)>0}">
                <c:set var="lValues" value="${fn:substring(lValues, 0, fn:length(lValues)-1)}"/>
            </c:if>            
            <sm-ep:limits title="statistics.counters.ca.label.limits" values="${lValues}"
                              first_field_name="statistics.counters.ca.label.condition"
                              second_field_name="statistics.counters.ca.label.action"
                              levels="<%=LevelType.getEvalableTypes()%>"/>

            <!-- Get parameters.-->
            <c:set var="pValues" value=""/>
            <c:forEach items="${bean.parameters}" var="parameter">
                <c:set var="pValues" value="${pValues}${parameter.name},${parameter.value};"/>
            </c:forEach>
            <c:if test="${fn:length(pValues)>0}">
                <c:set var="pValues" value="${fn:substring(pValues, 0, fn:length(pValues)-1)}"/>
            </c:if>
            <sm-ep:parameters title="tag.parameters" values="${pValues}"
                             first_field_name="tag.parameter.name" second_field_name="tag.parameter.value"/>
        </sm-ep:properties>
    </jsp:body>
</sm:page>