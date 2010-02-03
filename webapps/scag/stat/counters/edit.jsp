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
                        values="${fn:join(bean.typeIds, ',')}" valueTitles="${fn:join(bean.typeTitles, '~')}"/>
            <sm-ep:list title="statistics.counters.edit.list.ca"   name="ca" emptyOption="true"
                        values="${fn:join(bean.caIds, ',')}" valueTitles="${fn:join(bean.caTitles, '~')}"/>
        </sm-ep:properties>
    </jsp:body>
</sm:page>