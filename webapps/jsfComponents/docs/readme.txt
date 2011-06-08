Для работоспособности библиотеки компонентов jsf в приложении должны быть прописаны следующие настройки web.xml:

  <context-param>
    <param-name>org.apache.myfaces.trinidad.ALTERNATE_VIEW_HANDLER</param-name>
    <param-value>mobi.eyeline.util.jsf.components.AjaxViewHandler</param-value>
  </context-param>

  <servlet-mapping>
    <servlet-name>${JSF_SERVLET_NAME}</servlet-name>      <!-- ${JSF_SERVLET_NAME} - имя сервлета JSF-->
    <url-pattern>/faces/*</url-pattern>
  </servlet-mapping>

Пример приложения, использующего библиотеку компонентов, а также конфигурации web.xml и faces-config.xml можно посмотреть в sample.

В директории styles находится пример стилей компонентов.
Для формирования иного дизайна разработчик должен использовать названия классов css, указанных в примере.
