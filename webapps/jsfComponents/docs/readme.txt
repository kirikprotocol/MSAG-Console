��� ����������������� ���������� ����������� jsf � ���������� ������ ���� ��������� ��������� ��������� web.xml:

  <context-param>
    <param-name>org.apache.myfaces.trinidad.ALTERNATE_VIEW_HANDLER</param-name>
    <param-value>mobi.eyeline.util.jsf.components.AjaxViewHandler</param-value>
  </context-param>

  <servlet-mapping>
    <servlet-name>${JSF_SERVLET_NAME}</servlet-name>      <!-- ${JSF_SERVLET_NAME} - ��� �������� JSF-->
    <url-pattern>/faces/*</url-pattern>
  </servlet-mapping>

������ ����������, ������������� ���������� �����������, � ����� ������������ web.xml � faces-config.xml ����� ���������� � sample.

� ���������� styles ��������� ������ ������ �����������.
��� ������������ ����� ������� ����������� ������ ������������ �������� ������� css, ��������� � �������.
