package com.sptci.jnitest;
import static java.lang.String.format;
public class Child extends Base
{
  public int intMethod( int n ) { return n*n; }
  @Override
  public String stringMethod() { return format( "From %s.stringMethod", Child.class.getName() ); }
}