//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//  See the file COPYING for license details.
//
#ifndef _SCRIPTJDK_
#define _SCRIPTJDK_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_JDK
//
//  Java (via Sun JDK) Script class
//
#include "ScriptObject.h"

#include <jni.h>

#include "VrmlField.h"

class VrmlNodeScript;
class VrmlScene;

class ScriptJDK: public ScriptObject {

public:

  ScriptJDK( VrmlNodeScript *, const char *className, const char *classDir );
  ~ScriptJDK();

  virtual void activate( double timeStamp,
			 const char *fname,
			 int argc,
			 const VrmlField *argv[]);

  VrmlScene *browser();
  VrmlNodeScript *scriptNode() { return d_node; }

protected:

  // Shared by all JDK Script objects
  static JavaVM *d_jvm;
  static JNIEnv *d_env;

  // Per object data members
  VrmlNodeScript *d_node;

  jclass d_class;
  jobject d_object;
  jmethodID d_processEventsID, d_eventsProcessedID;

};

#endif // HAVE_JDK
#endif // _SCRIPTJDK_

