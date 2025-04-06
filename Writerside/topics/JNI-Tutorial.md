# JNI Tutorial

## Invoke Java API from C++


A very simple tutorial that illustrates accessing Java API from C++. We use a simple Java class hierarchy 
that exposes a static method as well as regular methods, and then develop a sample C++ 
client that instantiates the Java class and invokes the methods.

The workflow for accessing Java API is as follows:
* Create a JVM instance and specify the classpath as appropriate using `JNI_CreateJavaVM`
* Look up the Java Class definition using `JNIEnv::FindClass`
* Retrieve the constructor for the class using `JNIEnv::GetMethodID`
* Instantiate a new instance of the class using `JNIEnv::NewObject`
* Retrieve static methods using `JNIEnv::GetStaticMethodID` and instance methods using `JNIEnv::GetMethodID`
* Invoke static methods using `JNIEnv::CallStaticMethod` and instance methods using `JNIEnv::CallMethod` where result
  will depend upon the return type for the method (eg. Int, Boolean, Object, ...)
* You can invoke a super class variant of an instance method using `JNIEnv::CallNonvirtualMethod`.
  Note that the `jmethodID` must be retrieved using the appropriate super-class definition.

<tabs id="jni-tutorial-tabs">
  <tab title="Base Class" id="jni-tutorial-tabs-base">
    A simple base java class that exposes static and regular methods. We will be accessing instances of this class and its methods from our C++ client.<br/>
    <code>File: src/java/Base.java</code>
    <code-block lang="Java" src="jni/Base.java" collapsible="true"/>
  </tab>
  <tab title="Child Class" id="jni-tutorial-tabs-child">
    A simple child java class that regular methods as well as an over-ridden method. We will be accessing instances of this class and its methods from our C++ client.<br/>
    <code>File: src/java/Child.java</code>
    <code-block lang="Java" src="jni/Child.java" collapsible="true"/>
  </tab>
  <tab title="C++ Client" id="jni-tutorial-tabs-cppclient">
    A simple C++ client that starts a JVM, looks up our sample Java class, instantiates an instance and invokes the methods defined.<br/>
    <code>File: src/cpp/client.cpp</code>
    <code-block lang="C++" src="jni/client.cpp" collapsible="true"/>
  </tab>
  <tab title="Build and Run" id="jni-tutorial-tabs-build">
    A simple shell script used to build and run the samples. Note that we use javap to dump the method signatures for use in the C++ code.<br/>
    <code-block lang="shell" src="jni/build.sh" collapsible="true"/>
    A sample run should produce output similar to the following:
    <code-block lang="shell" collapsible="true">
init - Looking up CTOR for Child
init - Creating new Child instance
staticIntMethod - Result: 49
intMethod - Result: 25
booleanMethod - Result: 1
stringMethod - Result: From com.sptci.jnitest.Child.stringMethod
nonVirtualStringMethod - Result: From com.sptci.jnitest.Base.stringMethod
DTOR - Destroying JVM
Program Result code: 0
    </code-block>
  </tab>
  <tab title="Code Walk Through" id="jni-tutorial-tabs-code">
    Java developers familiar with the Reflection API will have noticed the similarities between
    using Reflection to look up a class, use constructor to instantiate an instance etc. and 
    the JNI API. They are indeed similar and follow a similar programming model.
    <h4>Start JVM</h4>
    The first step is to start a JVM instance with any parameters needed to properly initialise 
    the JVM (system properties, class path, ...). For efficiency C++ client applications will
    generally use a single JVM instance (encapsulated suitably using RAII) and use it to 
    load the Java API necessary. The process is illustrated in the `startJVM` method 
    of the Client class.
    <h4>Load Class and Instantiate Object</h4>
    The next step usually is to load a class available in the classpath using <code>JNIEnv::FindClass</code>.
    This step is equivalent to the <code>java.lang.Class.forName</code> static method. These steps 
    are illustrated in the <code>init</code> method of the Client class.<br/><br/>
    Once you have a <code>jclass</code> instance, you can invoke any static methods that 
    are defined for that class without need to instantiate an object instance. You can 
    look up constructors using the same technique as used to look up instance methods in 
    the class. Constructors are always identified by the name &lt;init&gt;. This is slightly
    different from Reflection which separates constructor lookups from method lookups.<br/><br/>
    You can instantiate an object using the <code>JNIEnv::NewObject</code> function, 
    which is the equivalent of <code>java.lang.reflect.Constructor.newInstance</code> method.
    <h4>Invoke Methods</h4>
    Static methods for a class are retrieved using <code>JNIEnv::GetStaticMethodID</code>,
    while instance methods are retrieved using <code>JNIEnv::GetMethodID</code>.
    Once you have a valid <code>jmethodID</code> you invoke the method using 
    <code>JNIEnv::CallStatic&lt;Type&gt;Method</code> or 
    <code>JNIEnv::Call&lt;Type&gt;Method</code> depending upon whether the method
    is static or not.
    <h5>Notes:</h5>
    In our sample Client class, we promote the <code>jclass</code> and <code>jobject</code>
    instances to global references to avoid the instances being garbage collected before 
    we are finished using them. Following RAII principles we release the references to 
    these instances in the destructor. JNI client applications will in general keep a 
    single global reference to Java class definitions and instantiate objects as
    necessary following the model used by the JVM.
  </tab>
</tabs>

* `com::sptci::jnitest::Client::staticIntMethod` - Invokes the static method defined in [Base.java](#jni-tutorial-tabs-base)
* `com::sptci::jnitest::Client::intMethod` - Invokes the instance method defined in [Child.java](#jni-tutorial-tabs-child)
* `com::sptci::jnitest::Client::booleanMethod` - Invokes the instance method defined in [Base.java](#jni-tutorial-tabs-base)
* `com::sptci::jnitest::Client::stringMethod` - Invokes the over-ridden method defined in [Child.java](#jni-tutorial-tabs-child)
* `com::sptci::jnitest::Client::nonVirtualStringMethod` - Invokes the base class variant of stringMethod defined in [Base.java](#jni-tutorial-tabs-base)

The code was built and tested on Mac OS X Mavericks. The shell script shown uses 
path's used on OS X, and will need to be modified accordingly for other platforms.
