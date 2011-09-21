package ru.novosoft.smsc.admin.route;

import org.junit.Test;

import static org.junit.Assert.*;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class RouteSubjectSettingsTest {

  @Test
  public void testContainsSubject() throws Exception {
    List<Subject> subjects = new ArrayList<Subject>();
    Collections.addAll(subjects, new Subject("123"), new Subject("456"));

    assertFalse(RouteSubjectSettings.containsSubject(subjects, "777"));
    assertTrue(RouteSubjectSettings.containsSubject(subjects, "123"));
    assertTrue(RouteSubjectSettings.containsSubject(subjects, "456"));
  }

  @Test
  public void testCheckUniqueOfSubjectNames() throws Exception {
    List<Subject> subjects = new ArrayList<Subject>();
    Collections.addAll(subjects, new Subject("123"), new Subject("456"));

    RouteSubjectSettings.checkUniqueOfSubjectNames(subjects);

    subjects.add(new Subject("123"));

    try {
      RouteSubjectSettings.checkUniqueOfSubjectNames(subjects);
      assertFalse(true);
    } catch (RouteException ignored) {
    }
  }

  @Test
  public void testCheckSubjectChilds() throws Exception {
    Subject subject1 = new Subject("subj1");
    subject1.setChildren(Arrays.asList("subj2"));

    Subject subject2 = new Subject("subj2");

    RouteSubjectSettings.checkSubjectChilds(Arrays.asList(subject1, subject2));
    RouteSubjectSettings.checkSubjectChilds(Arrays.asList(subject2));

    try {
      RouteSubjectSettings.checkSubjectChilds(Arrays.asList(subject1));
      assertTrue(false);
    } catch (RouteException ignored) {
    }
  }

  @Test
  public void testCheckNoCircles() throws Exception {
    Subject subject1 = new Subject("subj1");
    subject1.setChildren(Arrays.asList("subj2"));

    Subject subject2 = new Subject("subj2");

    RouteSubjectSettings.checkNoCircles(Arrays.asList(subject1, subject2));

    subject2.setChildren(Arrays.asList("subj1"));

    try {
      RouteSubjectSettings.checkNoCircles(Arrays.asList(subject1, subject2));
      assertTrue(false);
    } catch (RouteException ignored) {
    }
  }

  @Test
  public void testCheckRoutesSourcesAndDestinations() throws Exception {
    Route r1 = new Route("r1");
    r1.setSrcSmeId("srcSme");
    r1.setSources(Arrays.asList(new Source("subj1")));
    r1.setDestinations(Arrays.asList(new Destination("subj1", "DSTRLST")));

    RouteSubjectSettings.checkRoutesSourcesAndDestinations(Arrays.asList(r1), Arrays.asList(new Subject("subj1")));

    r1.setSrcSmeId(null);

    try {
      RouteSubjectSettings.checkRoutesSourcesAndDestinations(Arrays.asList(r1), Arrays.asList(new Subject("subj1")));
      assertTrue(false);
    } catch (RouteException ignored) {
    }

    r1.setSrcSmeId("srcSme");

    try {
      RouteSubjectSettings.checkRoutesSourcesAndDestinations(Arrays.asList(r1), Collections.<Subject>emptyList());
      assertTrue(false);
    } catch (RouteException ignored) {
    }

  }

  @Test
  public void testCheckUniqueOfRouteNames() throws Exception {
    RouteSubjectSettings.checkUniqueOfRouteNames(Arrays.asList(new Route("r1"), new Route("r2")));

    try {
      RouteSubjectSettings.checkUniqueOfRouteNames(Arrays.asList(new Route("r1"), new Route("r2"), new Route("r1")));
      assertTrue(false);
    } catch (RouteException ignored) {
    }
  }
}
