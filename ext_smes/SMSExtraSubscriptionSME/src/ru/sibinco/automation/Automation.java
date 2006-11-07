package ru.sibinco.automation;

import java.io.IOException;
import java.io.Writer;
import java.util.*;

/**
 * User: artem
 * Date: 23.10.2006
 */

final class Automation {
  public final State startState = new State();
  public final State finalState = new State();

  private final ArrayList statesLevels = new ArrayList();

  public StatesLevel newLevel() {
    final StatesLevel level = new StatesLevel();
    statesLevels.add(level);
    return level;
  }

  public StatesLevel getLevel(int index) {
    return (StatesLevel)statesLevels.get(index);
  }

  public int size() {
    final Set states = new HashSet();
    calculateSizeInternal(states, startState);
    return states.size();
  }

  private void calculateSizeInternal(Set statesSet, State st) {
    statesSet.add(st);

    for (Iterator iter = st.getOutLinks().keySet().iterator(); iter.hasNext();) {
      final char ch = ((Character)iter.next()).charValue();
      for (Iterator states = st.getOutLinks(ch).iterator(); states.hasNext();) {
        calculateSizeInternal(statesSet, (State)states.next());
      }
    }
  }

  public boolean allowWord(String word) {
    return allowWordRecursively(word, startState);
  }

  private boolean allowWordRecursively(String word, State state) {
    if (state == finalState && word.length() == 0)
      return true;
    if (word.length() == 0)
      return false;

    final Set nextStates = state.getOutLinks(word.charAt(0));
    if (nextStates == null)
      return false;

    final String subWord = word.substring(1);
    for (Iterator iter = nextStates.iterator(); iter.hasNext();) {
      if (allowWordRecursively(subWord, (State)iter.next()))
        return true;
    }
    return false;
  }

  public void save(Writer writer) throws IOException {
    // Clear all levels. They are unnecessary
    for (int i = 0; i< statesLevels.size(); i++)
      ((StatesLevel)statesLevels.get(i)).clear();
    statesLevels.clear();

    final Set statesSet = new HashSet();
    calculateSizeInternal(statesSet, startState);

    statesSet.remove(startState);

    final ArrayList states = new ArrayList();
    states.add(0, startState);
    states.addAll(statesSet);
    statesSet.clear();

    writer.write("  <automation>\n");
    writer.write("    <states count=\"" + states.size() + "\"/>\n");
    writer.write("    <links>\n");
    for (int i=0; i<states.size(); i++) {
      final State state = (State)states.get(i);
      writer.write("      <basestate id=\"" + i + "\">\n");

      for (Iterator outgoingChars = state.getOutgoingChars().iterator(); outgoingChars.hasNext();) {
        final char outgoingChar = ((Character)outgoingChars.next()).charValue();
        writer.write("        <char value=\"" + outgoingChar + "\">\n");

        for (Iterator outgoingStates = state.getOutLinks(outgoingChar).iterator(); outgoingStates.hasNext();)
          writer.write("          <state id=\"" + getStateNumberByState(states, (State)outgoingStates.next()) + "\"/>\n");

        writer.write("        </char>\n");
      }

      writer.write("      </basestate>\n");
    }
    writer.write("    </links>\n");
    writer.write("  </automation>\n");
  }

  private static int getStateNumberByState(ArrayList states, State state) {
    for (int i = 0; i < states.size(); i++) {
      if (state == states.get(i))
        return i;
    }
    return -1;
  }


  // ===================================================================================================================
  // ===================================================================================================================

  public final class StatesLevel {
    private final ArrayList states = new ArrayList();

    public void clear() {
      states.clear();
    }

    public void insertState(State state) {
      states.add(state);
    }

    private State getAllowedInState(State state, char ch, boolean isStartState) {
      final Set inLinks = state.getInLinks(ch);

      if (inLinks == null || inLinks.isEmpty())
        return null;
      for (Iterator iterator = inLinks.iterator(); iterator.hasNext();) {
        State st =  (State)iterator.next();
        if ((st != startState && !isStartState) || (st == startState && isStartState))
          return st;
      }
      return null;
    }

    public void insertState(State state, char outgoingChar, State outgoingState) {
      final State allowedInState = getAllowedInState(outgoingState, outgoingChar, state == startState);
      if (allowedInState != null) {
        states.add(allowedInState);
      } else {
        state.addOutLink(outgoingChar, outgoingState);
        outgoingState.addInLink(outgoingChar, state);
        states.add(state);
      }
    }

    private void joinStates(State fromState, State toState) {
      // Перекидываем исходящие ссылки fromState на toState
      {
        for (Iterator chars = fromState.getOutgoingChars().iterator(); chars.hasNext();) {
          final char ch = ((Character)chars.next()).charValue();

          final Set links = fromState.getOutLinks(ch);

          for (Iterator states = links.iterator(); states.hasNext();) {
            final State state = ((State)states.next());
            // Создаем новые ссылки в toState
            toState.addOutLink(ch, state);

            // Перекидываем incoming links у тех состояний, на которые ссылался fromState
            state.removeInLink(ch, fromState);
            state.addInLink(ch, toState);
          }

          // Очищаем память
          links.clear();
        }
      }

      // Перекидываем входящие в fromState ссылки на toState
      {
        // Берем состояния, ссылающиеся на fromState
        for (Iterator chars = fromState.getIncomingChars().iterator(); chars.hasNext();) {
          final char ch = ((Character)chars.next()).charValue();
          final Set links = fromState.getInLinks(ch);

          for (Iterator states = links.iterator(); states.hasNext();)
            ((State)states.next()).removeOutLink(ch, fromState);
//           Очищаем память
          links.clear();
        }
      }

      fromState.clearOutLinks();
      fromState.clearInLinks();
    }



    public void removeIterantStates() {
      for (int i=0; i < states.size(); i++) {
        final State state1 = getState(i);

        for (int j = states.size() - 1; j > i; j--) {
          final State state2 = getState(j);
          if (state1 == state2)
            states.remove(j);
        }
      }
    }

    public boolean processStates() {
      boolean levelModified = false;

      for (int i=0; i < states.size(); i++) {
        final State state1 = getState(i);

        for (int j = states.size() - 1; j > i; j--) {
          final State state2 = getState(j);
          if (state1 == state2) {
            states.remove(j);
          } else if (state1 != startState && state2 != startState && state1.compareInLinksWithState(state2)) {
            joinStates(state2, state1);
            states.remove(j);
            levelModified = true;
          }
        }
      }
      return levelModified;
    }

    public State getState(int index) {
      return (State)states.get(index);
    }

  }

}
