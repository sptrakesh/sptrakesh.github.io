#include <jni.h>
#include <iostream>
#include <string>

namespace com::sptci::jnitest
{
  class Client
  {
  public:
    ~Client()
    {
      if ( env && obj ) env->DeleteGlobalRef( obj );
      if ( env && child ) env->DeleteGlobalRef( child );

      if ( jvm )
      {
        std::cout << "DTOR - Destroying JVM" << std::endl;
        jvm->DestroyJavaVM();
      }
    }

    void run()
    {
      long vmstatus = startJVM();

      if ( vmstatus == JNI_ERR )
      {
        status = 1;
        return;
      }

      init();
      staticIntMethod();
      intMethod();
      booleanMethod();
      stringMethod();
      nonVirtualStringMethod();
    }

    int getStatus() { return status; }

  private:
    long startJVM()
    {
      JavaVMOption options[1];
      char str[] = "-Djava.class.path=.";
      options[0].optionString = str;

      JavaVMInitArgs vm_args;
      memset( &vm_args, 0, sizeof( vm_args ) );
      vm_args.version = JNI_VERSION_1_6;
      vm_args.nOptions = 1;
      vm_args.options = options;

      return JNI_CreateJavaVM( &jvm, reinterpret_cast<void**>( &env ), &vm_args );
    }

    void init()
    {
      child = env->FindClass( "com/sptci/jnitest/Child" );

      if ( ! child )
      {
        std::cerr << "init - Cannot find Child" << std::endl;
        status = 3;
        return;
      }
      child = static_cast<jclass>( env->NewGlobalRef( child ) );

      std::cout << "init - Looking up CTOR for Child" << std::endl;
      jmethodID mid = env->GetMethodID( child, "<init>", "()V" );

      if ( mid )
      {
        std::cout << "init - Creating new Child instance" << std::endl;
        obj = env->NewObject( child, mid );
        obj = env->NewGlobalRef( obj );
      }
      else
      {
        std::cerr << "Unable to find Child CTOR" << std::endl;
        status = 2;
      }
    }

    void staticIntMethod()
    {
      if ( ! child ) return;
      jmethodID mid = env->GetStaticMethodID( child, "staticIntMethod", "(I)I" );

      if ( mid )
      {
        jint square = env->CallStaticIntMethod( child, mid, 7 );
        std::cout << "staticIntMethod - Result: " << square << std::endl;
      }
    }

    void intMethod()
    {
      if ( ! obj ) return;
      jmethodID mid = env->GetMethodID( child, "intMethod", "(I)I" );

      if ( mid )
      {
        jint square = env->CallIntMethod( obj, mid, 5 );
        std::cout << "intMethod - Result: " << square << std::endl;
      }
    }

    void booleanMethod()
    {
      if ( ! obj ) return;
      jmethodID mid = env->GetMethodID( child, "booleanMethod", "(Z)Z" );

      if ( mid )
      {
        jboolean boolean = env->CallBooleanMethod( obj, mid, 1 );
        std::cout << "booleanMethod - Result: " << ( boolean ? JNI_TRUE : JNI_FALSE ) << std::endl;
      }
    }

    void stringMethod()
    {
      if ( ! obj ) return;
      jmethodID mid = env->GetMethodID( child, "stringMethod", "()Ljava/lang/String;" );

      if ( mid )
      {
        jstring str = static_cast<jstring>( env->CallObjectMethod( obj, mid ) );
        displayString( "stringMethod", str );
      }
    }

    void nonVirtualStringMethod()
    {
      jclass base = env->FindClass( "com/sptci/jnitest/Base" );

      if ( ! base || ! obj ) return;

      jmethodID mid = env->GetMethodID( base, "stringMethod", "()Ljava/lang/String;" );

      if ( mid )
      {
        jstring str = static_cast<jstring>( env->CallNonvirtualObjectMethod( obj, base, mid ) );
        displayString( "nonVirtualStringMethod", str );
      }

      env->DeleteLocalRef( base );
    }

    void displayString( std::string method, jstring str )
    {
        const char* cstr = env->GetStringUTFChars( str, nullptr );
        std::cout << method << " - Result: " << cstr << std::endl;
        env->ReleaseStringUTFChars( str, cstr );
        env->DeleteLocalRef( str );
    }

  private:
    JavaVM* jvm;
    JNIEnv* env;
    jclass child;
    jobject obj;
    int status = 0;
  };
}

int main()
{
  com::sptci::jnitest::Client client;
  client.run();
  return client.getStatus();
}