<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="Categories">
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbAdd" value="Add" title="Add new category"/>
            <sm-pm:item name="mbDelete" value="Delete" title="Delete category" onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
        <sm:table columns="checkbox,id,name" names="c,id,name" widths="1,30,30" edit="id"/>
    </jsp:body>
</sm:page>