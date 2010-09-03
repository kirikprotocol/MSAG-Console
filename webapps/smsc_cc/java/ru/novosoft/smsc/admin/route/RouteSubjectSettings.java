package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * @author Artem Snopkov
 */
public class RouteSubjectSettings implements Serializable {

  private static final transient ValidationHelper vh = new ValidationHelper(RouteSubjectSettings.class);

  private List<Subject> subjects = new ArrayList<Subject>();
  private List<Route> routes = new ArrayList<Route>();
  private final transient RouteSubjectManager manager;

  RouteSubjectSettings(RouteSubjectManager manager) {
    this.manager = manager;
  }

  RouteSubjectSettings(RouteSubjectSettings copy) {
    this.manager = copy.manager;
    this.subjects.addAll(copy.subjects);
    this.routes.addAll(copy.routes);
  }

  /**
   * Производит трассировку маршрута т.е. проверку наличия маршрута для указанных отправителя и получателя
   * @param source адрес отправителя
   * @param destination адрес получателя
   * @param sourceSmeId smeId отправителя
   * @return объект RouteTrace с результатами проверки
   * @throws AdminException если произошла ошибка
   */
  public RouteTrace traceRoute(Address source, Address destination, String sourceSmeId) throws AdminException {
    return manager.traceRoute(this, source, destination, sourceSmeId);    
  }

  /**
   * Возвращает список субъектов
   *
   * @return список субъектов
   */
  public List<Subject> getSubjects() {
    return new ArrayList<Subject>(subjects);
  }

  /**
   * Проверяет наличие в списке subjects субъекта с именем subjectName
   *
   * @param subjects    список, в котором надо искать
   * @param subjectName имя, которое надо найти
   * @return true, если субъект найден, иначе false
   */
  static boolean containsSubject(List<Subject> subjects, String subjectName) {
    for (Subject s : subjects)
      if (s.getName().equals(subjectName))
        return true;
    return false;
  }

  /**
   * Проверяет, что имена субъектов в списке уникальны
   *
   * @param subjects список субъектов, которые надо проверить
   * @throws RouteException если список соержит по крайней мере два субъекта с одинаковыми именами
   */
  static void checkUniqueOfSubjectNames(List<Subject> subjects) throws RouteException {
    for (int i = 0; i < subjects.size(); i++) {
      String subjName = subjects.get(i).getName();
      for (int j = i + 1; j < subjects.size(); j++) {
        if (subjects.get(j).getName().equals(subjName))
          throw new RouteException("subject.name.is.not.unique", subjName);
      }
    }
  }

  /**
   * Проверяет, что все потомки всех субъектов из списка subjects существуют в этом списке
   *
   * @param subjects список субъектов, которые надо проверить
   * @throws RouteException если в списке существует субъект с потомком, которого нет в этом списке
   */
  static void checkSubjectChilds(List<Subject> subjects) throws RouteException {
    for (Subject s : subjects) {
      List<String> children = s.getChildren();
      if (children != null) {
        for (String child : children) {
          if (!containsSubject(subjects, child))
            throw new RouteException("child.subject.not.exists", child);
        }
      }
    }
  }

  /**
   * Проверяет список субъектов на наличие циклов
   *
   * @param subjects список субъектов, которые надо проверить
   * @throws RouteException если список содержит циклы
   */
  static void checkNoCircles(List<Subject> subjects) throws RouteException {
    // Try to find circles in subjects graph
    // Construct graph matrix
    final int[][] matrix = new int[subjects.size()][subjects.size()];
    for (int i = 0; i < matrix.length; i++) {
      for (int j = 0; j < matrix.length; j++)
        matrix[i][j] = 0;
    }
    final ArrayList<String> subjectNames = new ArrayList<String>();
    for (Subject s : subjects)
      subjectNames.add(s.getName());

    // Fill graph matrix

    for (Subject subj : subjects) {
      int i = subjectNames.indexOf(subj.getName());
      List<String> childs = subj.getChildren();
      if (childs != null) {
        for (String child : childs) {
          int k = subjectNames.indexOf(child);
          matrix[i][k] = 1;
        }
      }
    }

    // Find circles

    // Find non circled elements
    boolean removed = true;
    final Set<Integer> removedVertexes = new HashSet<Integer>();
    while (removed) {
      removed = false;
      for (int i = 0; i < matrix.length; i++) {
        if (removedVertexes.contains(i)) // vertex was removed
          continue;

        // Check outgoing links
        boolean found = false;
        for (int j = 0; j < matrix.length && !found; j++)
          found = matrix[i][j] > 0;

        if (found) {
          // Check incoming links
          found = false;
          for (int j = 0; j < matrix.length && !found; j++)
            found = matrix[j][i] > 0;

          if (found) // Outgoing and incoming links was found
            continue;
        }

        // Vertex has just outgoing or just incoming links. Remove it.
        for (int j = 0; j < matrix.length; j++) {
          matrix[i][j] = -1;
          matrix[j][i] = -1;
        }
        removedVertexes.add(i);
        removed = true;
      }
    }

    for (int i = 0; i < matrix.length; i++) {
      for (int j = 0; j < matrix.length; j++)
        if (matrix[i][j] > 0) {
          final StringBuffer buffer = new StringBuffer();
          printCircle(matrix, subjectNames, i, j, buffer, new HashSet<Integer>());
          throw new RouteException("subjects.contains.circle", buffer.toString());
        }
    }
  }

  private static void printCircle(int[][] matrix, ArrayList<String> subjectNames, int i, int j, StringBuffer buffer, Set<Integer> steps) {
    if (steps.contains(i))
      return;

    steps.add(i);
    buffer.append(buffer.length() > 0 ? " -> " : "").append(subjectNames.get(i));

    for (int k = 0; k < matrix.length; k++) {
      if (matrix[j][k] > 0) {
        printCircle(matrix, subjectNames, j, k, buffer, steps);
        return;
      }
    }
  }

  /**
   * Проверяет для каждого маршрута из routes корректность отправителей и получателей
   *
   * @param routes   список маршрутов
   * @param subjects список субъектов
   * @throws RouteException если существует маршрут, в котором есть субъект, не содержащийся в subjects
   */
  static void checkRoutesSourcesAndDestinations(List<Route> routes, List<Subject> subjects) throws RouteException {
    for (Route r : routes) {
      if (r.getSources() == null)
        throw new RouteException("route.has.no.sources", r.getName());
      for (Source src : r.getSources()) {
        if (src.getSubject() != null && !containsSubject(subjects, src.getSubject()))
          throw new RouteException("route.contains.unknown.subject", r.getName() + ", " + src.getSubject());
      }

      if (r.getDestinations() == null)
        throw new RouteException("route.has.no.destinations", r.getName());
      for (Destination dst : r.getDestinations()) {
        if (dst.getSubject() != null && !containsSubject(subjects, dst.getSubject()))
          throw new RouteException("route.contains.unknown.subject", r.getName() + ", " + dst.getSubject());
      }
    }
  }

  /**
   * Задает новый список субъектов
   *
   * @param subjects новый список субъектов
   * @throws AdminException если список субъектов содержит некорректные данные
   */
  public void setSubjects(List<Subject> subjects) throws AdminException {
    vh.checkNoNulls("subjects", subjects);
    checkUniqueOfSubjectNames(subjects);
    checkSubjectChilds(subjects);
    checkNoCircles(subjects);
    checkRoutesSourcesAndDestinations(routes, subjects);

    this.subjects = new ArrayList<Subject>(subjects);
  }

  /**
   * Возвращает список маршрутов
   *
   * @return список маршрутов
   */
  public List<Route> getRoutes() {
    return new ArrayList<Route>(routes);
  }

  /**
   * Проверяет список маршрутов на уникальность имен
   *
   * @param routes список маршрутов, которые надо проверить
   * @throws RouteException еси список содержит по крайней мере 2 маршрута с одинаковыми именами
   */
  static void checkUniqueOfRouteNames(List<Route> routes) throws RouteException {
    for (int i = 0; i < routes.size(); i++) {
      String routeName = routes.get(i).getName();
      for (int j = i + 1; j < routes.size(); j++) {
        if (routes.get(j).getName().equals(routeName))
          throw new RouteException("route.name.is.not.unique", routeName);
      }
    }
  }

  /**
   * Задает новый список маршрутов
   *
   * @param routes новый список маршрутов
   * @throws AdminException если список маршрутов содержит некорректные данные
   */
  public void setRoutes(List<Route> routes) throws AdminException {
    vh.checkNoNulls("routes", routes);
    checkUniqueOfRouteNames(routes);
    checkRoutesSourcesAndDestinations(routes, subjects);

    this.routes = new ArrayList<Route>(routes);
  }

  /**
   * Возвращает копию RouteSubjectSettings
   *
   * @return копию RouteSubjectSettings
   */
  public RouteSubjectSettings cloneSettings() {
    return new RouteSubjectSettings(this);
  }
}
