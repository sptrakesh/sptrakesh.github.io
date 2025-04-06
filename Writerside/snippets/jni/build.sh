#!/bin/ksh

javac -d build src/java/*.java
javap -cp build -s -p com.sptci.jnitest.Base
javap -cp build -s -p com.sptci.jnitest.Child

clang++ -std=c++11 \
  -I$JAVA_HOME/include -I$JAVA_HOME/include/darwin \
  -L$JAVA_HOME/jre/lib/server -ljvm \
  -rpath $JAVA_HOME/jre/lib/server \
  -o build/Client src/cpp/Client.cpp

if [ $? -eq 0 ]
then
  (cd build; ./Client; echo "Program Result code: $?")
else
  echo "Build failed"
fi