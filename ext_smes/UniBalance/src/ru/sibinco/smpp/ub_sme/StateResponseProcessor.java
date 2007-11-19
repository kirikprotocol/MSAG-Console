package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.PDU;

/**
 * Created by pasha
 * Date: 12.11.2007
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public interface StateResponseProcessor {
    public void handleResponse(PDU pdu, State state);
}
