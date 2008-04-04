<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page>
    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">services.add.title</c:when>
            <c:otherwise>services.edit.title</c:otherwise>
        </c:choose>
    </jsp:attribute>

    <jsp:attribute name="menu">

    </jsp:attribute>
    <jsp:body>
        <sm-ep:properties title="services.edit.properties">
            <c:if test="${!bean.add}"><input type="hidden" name="id" id="id" value="${fn:escapeXml(bean.id)}"></c:if>
            <sm-ep:txt title="services.edit.txt.name" name="name" validation="nonEmpty"/>
            <sm-ep:txtBox title="services.edit.txtbox.description" cols="0" rows="0" name="description"/>
        </sm-ep:properties>
        <div class=page_subtitle>&nbsp;</div>
        <c:if test="${!param.add}">
        <sm-pm:menu>
                <sm-pm:item name="mbSave" value="services.edit.item.mbsave.value" title="services.edit.item.mbsave.title"/>
                <sm-pm:item name="mbCancel" value="services.edit.item.mbcancel.value" title="services.edit.item.mbcancel.title" onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
        </c:if>
        <c:if test="${!param.add}">
            <div class=page_subtitle>&nbsp;</div>

            <div class=page_subtitle><fmt:message>services.edit.label.services_list</fmt:message></div>
<%--            <sm:table columns="checkbox,id,name" names="c,services.edit.table.names.id,services.edit.table.names.name,services num" widths="1,20" child="/service" edit="name"/>--%>
            <sm:table parentId="${bean.id}" columns="checkbox,id,name"
                      names="c,services.edit.table.names.id,services.edit.table.names.name,services num"
                      widths="1,20" child="/service" edit="name"/>
        </c:if>
        <div class=page_subtitle>&nbsp;</div>
        <sm-pm:menu>
            <c:if test="${!param.add}">
                <sm-pm:item name="mbAddChild" value="services.edit.item.mbaddchild.value" title="services.edit.item.mbaddchild.title"/>
            </c:if>
            <c:if test="${!param.add}">
                <sm-pm:item name="mbDelete" value="services.edit.item.mbdelete.value" title="services.edit.item.mbdelete.title"
                            onclick="return deleteConfirm()" isCheckSensible="true"/>
            </c:if>
            <c:if test="${param.add}">
                <sm-pm:item name="mbSave" value="services.edit.item.mbsave2.value" title="services.edit.item.mbsave2.title"/>
                <sm-pm:item name="mbCancel" value="services.edit.item.mbcancel.value" title="services.edit.item.mbcancel.title" onclick="clickCancel()"/>
            </c:if>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:body>
</sm:page>