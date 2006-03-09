<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page>
    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">Create new Service</c:when>
            <c:otherwise>Edit Service</c:otherwise>
        </c:choose>
    </jsp:attribute>

    <jsp:attribute name="menu">
        <sm-pm:menu>
            <c:if test="${!param.add}">
                <sm-pm:item name="mbSave" value="Done" title="Save Service info"/>
            </c:if>
            <c:if test="${param.add}">
                <sm-pm:item name="mbSave" value="Create" title="Create Service info"/>
            </c:if>
            <sm-pm:item name="mbCancel" value="Cancel" title="Cancel Service editing" onclick="clickCancel()"/>
            <c:if test="${!param.add && bean.delete}">
                <sm-pm:item name="mbDelete" value="Delete" title="Delete Service"
                            onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
            </c:if>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
        <sm-ep:properties title="Service info">
            <c:if test="${!bean.add}"><input type="hidden" name="id" id="id" value="${fn:escapeXml(bean.id)}"></c:if>
            <sm-ep:txt title="name" name="name" validation="nonEmpty"/>
            <sm-ep:txtBox title="description" cols="0" rows="0" name="description"/>
            <input type="hidden" id="parentId" name="parentId" value="${fn:escapeXml(bean.parentId)}">

        </sm-ep:properties>

        <applet code="org.gjt.sp.jedit.jEdit.class" codebase="rules/rules" width="1" height="1" archive="jedit.jar" name=jedit ID=jedit>
            <param name="noplugins" value="-noplugins">
            <param name="homedir" value="applet">
            <param name="username" value="rules">
            <param name="servletUrl" value="/scag/applet/myServlet">
            <param name="targetUrl" value="/scag/services/service/edit.jsp?editId=${bean.editId}&parentId=${bean.parentId}&editChild=true">
        </applet>

        <hr/>
          <sm-ep:properties title="Service rule">
            <sm:rule transport="SMPP" exists="${bean.smppRuleExists}" />

            <sm:rule transport="HTTP" exists="${bean.httpRuleExists}" />

            <sm:rule transport="MMS" exists="${bean.mmsRuleExists}" />
          </sm-ep:properties>
       <hr/>

        <c:if test="${!param.add}">
            <sm:table columns="checkbox,name" names="c,name" widths="1,20" edit="name"/>
        </c:if>
    </jsp:body>
</sm:page>