<%@include file="/WEB-INF/inc/header.jspf"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<sm:page>
    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">statistics.counters.add.title</c:when>
            <c:otherwise>statistics.counters.edit.title</c:otherwise>
        </c:choose>
    </jsp:attribute>

    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbSave"   value="statistics.counters.edit.item.mbsave.value"
                                        title="statistics.counters.edit.item.mbsave.title"/>
            <sm-pm:item name="mbCancel" value="statistics.counters.edit.item.mbcancel.value"
                                        title="statistics.counters.edit.item.mbcancel.title"
                                        onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    
    <jsp:body>        
        <sm-ep:properties title="statistics.counters.edit.properties">
            <sm-ep:txt title="statistics.counters.edit.txt.id"     name="id" validation="nonEmpty" readonly="${!bean.add}"/>
            <sm-ep:list title="statistics.counters.edit.list.type" name="type" readonly="${!bean.add}" emptyOption="true"
                        values="${fn:join(bean.typeTitles, ',')}" valueTitles="${fn:join(bean.typeTitles, ',')}"/>
            <sm-ep:list title="statistics.counters.edit.list.ca"   name="CATableId" emptyOption="true"
                        values="${fn:join(bean.caIds, ',')}" valueTitles="${fn:join(bean.caIds, ',')}"/>


            <script src="content/scripts/counters.js" type="text/javascript"></script>
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