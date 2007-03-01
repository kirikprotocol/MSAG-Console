/*
 * @(#)RegisterAttemptAnswer.java 
 * Created on: 28.02.2007
 * 
 * $Revision$, $Date$
 */
package ru.sibinco.smpp.cmb;

/**
 * @author  Alexander Shirkov
 * @version $Revision$, $Date$
 */
public class RegisterAttemptAnswer {

    private int value;
    private boolean isUsagesDecreased;

    /**
     * @param value
     * @param isUsagesDecreased
     */
    public RegisterAttemptAnswer(int value, boolean isUsagesDecreased) {
        super();
        this.value = value;
        this.isUsagesDecreased = isUsagesDecreased;
    }

    public boolean isUsagesDecreased() {
        return isUsagesDecreased;
    }

    public void setUsagesDecreased(boolean isUsagesDecreased) {
        this.isUsagesDecreased = isUsagesDecreased;
    }

    public int getValue() {
        return value;
    }
}
