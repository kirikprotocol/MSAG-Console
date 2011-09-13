Для работоспособности библиотеки компонентов jsf в приложении должны быть прописаны следующие настройки web.xml:

  <context-param>
    <param-name>org.apache.myfaces.trinidad.ALTERNATE_VIEW_HANDLER</param-name>
    <param-value>mobi.eyeline.util.jsf.components.TrinidadAlternateViewHandler</param-value>
  </context-param>

Пример приложения, использующего библиотеку компонентов, а также конфигурации web.xml и faces-config.xml можно посмотреть в sample.

В директории styles находится пример стилей компонентов.
Для формирования иного дизайна разработчик должен использовать названия классов css, указанных в примере.
