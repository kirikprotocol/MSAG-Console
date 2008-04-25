/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.33
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.eyeline.jstore;

public class JStore4Java {

  static {
    try {
      System.loadLibrary("JStore4Java");
    } catch (UnsatisfiedLinkError e) {
      System.out.println("JStore4Java library not found");
      e.printStackTrace();
    }
  }

  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected JStore4Java(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(JStore4Java obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      JStoreJNI.delete_JStore4Java(swigCPtr);
    }
    swigCPtr = 0;
  }

  public void Init(String fn, int rt, int mxc) {
    JStoreJNI.JStore4Java_Init(swigCPtr, this, fn, rt, mxc);
  }

  public void Insert(long k, int v) {
    JStoreJNI.JStore4Java_Insert(swigCPtr, this, k, v);
  }

  public void Delete(long k) {
    JStoreJNI.JStore4Java_Delete(swigCPtr, this, k);
  }

  public boolean Lookup(long k, int[] vptr) {
    return JStoreJNI.JStore4Java_Lookup(swigCPtr, this, k, vptr);
  }

  public void setWaitRolling(boolean val) {
    JStoreJNI.JStore4Java_setWaitRolling(swigCPtr, this, val);
  }

  public void forceRoll() {
    JStoreJNI.JStore4Java_forceRoll(swigCPtr, this);
  }

  public void Stop() {
    JStoreJNI.JStore4Java_Stop(swigCPtr, this);
  }

  public void First() {
    JStoreJNI.JStore4Java_First(swigCPtr, this);
  }

  public boolean Next(long[] keyptr) {
    return JStoreJNI.JStore4Java_Next(swigCPtr, this, keyptr);
  }

  public JStore4Java() {
    this(JStoreJNI.new_JStore4Java(), true);
  }

}
