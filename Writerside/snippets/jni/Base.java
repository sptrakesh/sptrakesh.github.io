package com.sptci.jnitest;
import static java.lang.String.format;
public class Base
{
  public static int staticIntMethod( int n ) { return n*n; }
  public boolean booleanMethod( boolean bool ) { return bool; }
  public String stringMethod() { return format( "From %s.stringMethod", Base.class.getName() ); }
}