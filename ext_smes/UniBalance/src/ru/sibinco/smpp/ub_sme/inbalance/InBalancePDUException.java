package ru.sibinco.smpp.ub_sme.inbalance;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class InBalancePDUException extends Exception {
    public InBalancePDUException(String message) {
        super(message);
    }

    public InBalancePDUException(String message, Throwable cause) {
        super(message, cause);
    }

}
