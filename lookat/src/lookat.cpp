//
//  OpenGL/Glut Vrml97 browser using LibVRML97.
//  Copyright (C) 1998 Chris Morley
//
//  Program to exercise the VrmlScene and ViewerGlut classes.
//

#include <GL/glut.h>
#include <vrml97/Doc.h>
#include <vrml97/System.h>
#include <vrml97/VrmlScene.h>

#include "ViewerGlut.h"

static void worldChangedCB(int);
VrmlScene *vrmlScene = 0;
ViewerGlut   *viewer = 0;


int
main(int argc, char **argv)
{
  glutInitWindowSize(400, 320);
  glutInit( &argc, argv);

  char *inputUrl = 0;
  char *inputName = 0;
  char *outputName = 0;

  char usage[] = " file.wrl [outputfile]\n";

  for (int i = 1; i < argc; ++i)
    {
      if (*argv[i] == '-')
	{
	  if (strcmp(argv[i], "-url") == 0)
	    inputUrl = argv[++i];
	  else
	    {
	      cerr << "Error: unrecognized option " << argv[i] << '\n';
	      cerr << "Usage: " << argv[0] << usage << endl;
	      exit(1);
	    }
	}
      else if (! inputName)
	inputName = argv[i];
      else if (! outputName)
	outputName = argv[i];
      else
	{
	  cerr << "Usage: " << argv[0] << usage << endl;
	  exit(1);
	}
    }

  if (! inputName)
    {
      if (inputUrl)
	inputName = inputUrl;
      else
	inputName = inputUrl = "-";		// Read stdin
    }
  else
    cout << "Loading " << inputName << " ...";

  if (! inputUrl) inputUrl = inputName;

  vrmlScene = new VrmlScene( inputUrl, inputName );

  if (outputName)
    {
      cout << "  Saving scene to " << outputName << endl;
      if (! vrmlScene->save(outputName))
	cout << "\nError: couldn't write to " << outputName << endl;
    }

  viewer = new ViewerGlut( vrmlScene );
  if (! viewer)
    {
      cout << "\nError: couldn't create GLUT viewer.\n";
      exit(1);
    }

  if (*inputName != '-')
    cout << " done.\n";

  vrmlScene->addWorldChangedCallback( worldChangedCB );
  worldChangedCB( VrmlScene::REPLACE_WORLD );
  viewer->update();

  glutMainLoop();
  return 0;
}


static void worldChangedCB(int reason)
{
  switch (reason)
    {
    case VrmlScene::DESTROY_WORLD:
      delete viewer;
      delete vrmlScene;
      exit(0);
      break;

    case VrmlScene::REPLACE_WORLD:
      Doc *urlDoc = vrmlScene->urlDoc();
      if (urlDoc)
	{
	  const char *title = urlDoc->urlBase();
	  if (title && *title) glutSetWindowTitle(title);
	}
      break;
    }
}
