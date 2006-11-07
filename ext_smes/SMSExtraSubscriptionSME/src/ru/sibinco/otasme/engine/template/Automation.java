package ru.sibinco.otasme.engine.template;

/**
 * User: artem
 * Date: 25.10.2006
 */

public final class Automation {

  private State startState;

  public void setStartState(State startState) {
    this.startState = startState;
  }

  public boolean allowWord(String word) {
    return allowWordRecursively(word, startState);
  }

  private boolean allowWordRecursively(String word, State state) {
    if (state.isFinal())
      return true;
//    System.out.println("check: " + word.charAt(0));
    final State[] nextStates = state.getOutLinks(word.charAt(0));
    if (nextStates == null)
      return false;

    if (word.length() >= 1) {
      final String subWord = word.substring(1);
      for (int i=0; i<nextStates.length; i++) {
        if (allowWordRecursively(subWord, nextStates[i]))
          return true;
      }
    }

    return false;
  }

}
