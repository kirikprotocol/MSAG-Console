package com.sibinco.soap_balance.service;
/**
 * Copyright (c) 
 * EyeLine Communications
 * All rights reserved.
 */

/**
 * Created by Silvestrov Ilya
 * Date: Jun 28, 2007
 * Time: 12:23:11 PM
 */
public class SoapService implements BalanceService {
  public double getBalance(String abonent) {
    return 10.0*Math.random();
  }
}
