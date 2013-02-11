/*
 * appmech.cpp
 *
 * Definition of AppMech, the main application object.
 */

#include "appmech.h"
#include <cstool/simplestaticlighter.h>
#include <celtool/initapp.h>
#include <celtool/persisthelper.h>

// Convenience function to get a csVector3 from a config file.
static csVector3 GetConfigVector (iConfigFile* config,
        const char* key, const char* def)
{
  csVector3 v;
  csScanStr (config->GetStr (key, def), "%f,%f,%f", &v.x, &v.y, &v.z);
  return v;
}

AppMech::AppMech() : csApplicationFramework()
{
  SetApplicationName("mech");
  movement = csVector3(0);
  speed = 0;
}

AppMech::~AppMech()
{
}

void AppMech::OnExit()
{
  if (pl) pl->CleanCache ();
  printer.Invalidate();
}

void AppMech::CaptureInputState()
{
  csVector3 obj_move (0);
  csVector3 obj_rotate (0);
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  csTicks current_time = vc->GetCurrentTicks ();
  csVector3 bounce (0, 1, 0);
  float factor = (current_time % 5000) / 5000.0f;

  // mc->calculateMovement(elapsed_time, obj_move&, obj_rotate&);

  // iObjectRegistry* r = GetObjectRegistry();
  // iKeyboardDriver* kbd = csQueryRegistry<iKeyboardDriver> (r);
  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the arrow keys will cause
    // the camera to strafe up, down, left or right from it's
    // current position.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      obj_move = CS_VEC_RIGHT * 3.0f;
    if (kbd->GetKeyState (CSKEY_LEFT))
      obj_move = CS_VEC_LEFT * 3.0f;
    if (kbd->GetKeyState (CSKEY_UP))
      obj_move = CS_VEC_UP * 3.0f;
    if (kbd->GetKeyState (CSKEY_DOWN))
      obj_move = CS_VEC_DOWN * 3.0f;
  }
  else
  {
    // left and right cause the camera to rotate on the global Y
    // axis; page up and page down cause the camera to rotate on the
    // _camera's_ X axis (more on this in a second) and up and down
    // arrows cause the camera to go forwards and backwards.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      obj_rotate.Set (0, 1, 0);
    if (kbd->GetKeyState (CSKEY_LEFT))
      obj_rotate.Set (0, -1, 0);
    if (kbd->GetKeyState (CSKEY_PGUP))
      obj_rotate.Set (1, 0, 0);
    if (kbd->GetKeyState (CSKEY_PGDN))
      obj_rotate.Set (-1, 0, 0);
    if (kbd->GetKeyState (CSKEY_UP)) {
      speed += .1;
    }
    if (kbd->GetKeyState (CSKEY_DOWN)) {
      speed -= .1;
    }
    movement = CS_VEC_FORWARD * speed;
  }

  iObjectRegistry* r = GetObjectRegistry();
  csRef<iMouseDriver> mouse = csQueryRegistry<iMouseDriver> (r);
  iGraphics2D* g2d = g3d->GetDriver2D ();
  int x = mouse->GetLastX ();
  int y = mouse->GetLastY ();
  int centerX = g2d->GetWidth () / 2;
  int centerY = g2d->GetHeight () / 2;
  int diffX = x - centerX;
  int diffY = y - centerY;
  obj_rotate += csVector3(-diffY / 10.f, diffX / 10.f, 0);
  g2d->SetMousePosition (centerX, centerY);

  csVector3 currentBounce = bounce * (factor * movement.Norm());
  obj_move = obj_move + movement + currentBounce;

  printf("%f %f - %s\n", factor, movement.Norm(), currentBounce.Description().GetData());


  // To get the camera/actor position, you can use that:
  iCamera* cam = view->GetCamera ();
  csVector3 pos (cam->GetTransform ().GetOrigin ());
  printf("%s\n", pos.Description().GetData());

  collider_actor.Move (float (elapsed_time) / 1000.0f,
                       1.0f,
                       obj_move,
                       obj_rotate);

}

void AppMech::Frame()
{
  CaptureInputState();

  if (g3d->BeginDraw(CSDRAW_3DGRAPHICS))
  {
    // Draw frame.
    view->Draw ();
  }
}

bool AppMech::OnKeyboard(iEvent& ev)
{
  // We got a keyboard event.
  if (csKeyEventHelper::GetEventType(&ev) == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC)
    {
      // The user pressed escape, so terminate the application.  The proper way
      // to terminate a Crystal Space application is by broadcasting a
      // csevQuit event.  That will cause the main run loop to stop.  To do
      // so we retrieve the event queue from the object registry and then post
      // the event.
      csRef<iEventQueue> q =
        csQueryRegistry<iEventQueue> (GetObjectRegistry());
      if (q.IsValid())
        q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
    }
  }
  return false;
}

bool AppMech::OnInitialize(int argc, char* argv[])
{
  iObjectRegistry* r = GetObjectRegistry();

  // Load application-specific configuration file.
  if (!csInitializer::SetupConfigManager(r,
      "/mech/AppMech.cfg", GetApplicationName()))
    return ReportError("Failed to initialize configuration manager!");

  celInitializer::SetupCelPluginDirs(r);

  // RequestPlugins() will load all plugins we specify.  In addition it will
  // also check if there are plugins that need to be loaded from the
  // configuration system (both the application configuration and CS or global
  // configurations).  It also supports specifying plugins on the command line
  // via the --plugin= option.
  if (!csInitializer::RequestPlugins(r,
	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_PLUGIN ("cel.physicallayer", iCelPlLayer),
	CS_REQUEST_PLUGIN("crystalspace.collisiondetection.opcode",
		iCollideSystem),
	CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
	CS_REQUEST_PLUGIN ("crystalspace.sndsys.element.loader", iSndSysLoader),
	CS_REQUEST_PLUGIN ("crystalspace.sndsys.renderer.software",
		iSndSysRenderer),
	CS_REQUEST_END))
	// CS_REQUEST_PLUGIN ("cel.persistence.xml", iCelPersistence),
	// CS_REQUEST_PLUGIN ("crystalspace.device.joystick", iEventPlug),
    return ReportError("Failed to initialize plugins!");

  // "Warm up" the event handler so it can interact with the world
  csBaseEventHandler::Initialize(GetObjectRegistry());

  // Set up an event handler for the application.  Crystal Space is fully
  // event-driven.  Everything (except for this initialization) happens in
  // response to an event.
  if (!RegisterQueue (r, csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

bool AppMech::Application()
{
  iObjectRegistry* r = GetObjectRegistry();

  // Open the main system. This will open all the previously loaded plugins
  // (i.e. all windows will be opened).
  if (!OpenApplication(r))
    return ReportError("Error opening system!");

  // Now get the pointer to various modules we need.  We fetch them from the
  // object registry.  The RequestPlugins() call we did earlier registered all
  // loaded plugins with the object registry.  It is also possible to load
  // plugins manually on-demand.
  g3d = csQueryRegistry<iGraphics3D> (r);
  if (!g3d)
    return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (r);
  if (!engine)
    return ReportError("Failed to locate 3D engine!");

  printer.AttachNew(new FramePrinter(GetObjectRegistry()));

  vc = csQueryRegistry<iVirtualClock> (r);
  if (!vc)
    return ReportError ("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (r);
  if (!kbd)
    return ReportError ("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (r);
  if (!loader)
    return ReportError("Failed to locate map loader plugin!");

  cdsys = csQueryRegistry<iCollideSystem> (r);
  if (!cdsys)
    return ReportError ("Failed to locate CD system!");

  cfgmgr = csQueryRegistry<iConfigManager> (r);
  if (!cfgmgr)
    return ReportError ("Failed to locate the configuration manager plugin!");

  cegui = csQueryRegistry<iCEGUI> (r);
  if (!cegui)
    return ReportError ("Failed to locate the CEGUI plugin!");

  pl = csQueryRegistry<iCelPlLayer> (object_reg);
  if (!pl)
    return ReportError ("CEL physical layer missing!");

  SetupCegui();
  SetupWorld();

  if (!LoadMap (cfgmgr->GetStr ("DefaultMap", "/appdata/defaultmap")))
    return ReportError("Failed to load map!");

  if (!PostLoadMap ())
    return ReportError ("Error during PostLoadMap()!");

  Run();

  return true;
}

void AppMech::SetupWorld()
{
  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();

  // Set the window title.
  iNativeWindow* nw = g2d->GetNativeWindow ();
  if (nw)
    nw->SetTitle (cfgmgr->GetStr ("WindowTitle",
          "Please set WindowTitle in AppMech.cfg"));


  int height = g2d->GetHeight ();
  view->SetRectangle (0, height / 2, g2d->GetWidth (), height);
  iPerspectiveCamera* cam = view->GetPerspectiveCamera();
  cam->SetFOVAngle(120, 2);
}

void AppMech::SetupCegui()
{
  cegui->Initialize();
  cegui->SetAutoRender(true);
  cegui->GetLoggerPtr ()->setLoggingLevel(CEGUI::Informative);

  // Set VFS current directory to the level we want to load.
  csRef<iVFS> vfs (csQueryRegistry<iVFS> (GetObjectRegistry ()));
  vfs->ChDir ("/cegui/");

  // Load the ice skin (which uses Falagard skinning system)
  cegui->GetSchemeManagerPtr ()->create("SleekSpace.scheme");

  cegui->GetSystemPtr ()->setDefaultMouseCursor("SleekSpace", "MouseArrow");
  // cegui->GetMouseCursorPtr ()->hide();

  cegui->GetFontManagerPtr ()->createFreeTypeFont("DejaVuSans", 10, true, "/fonts/ttf/DejaVuSans.ttf");

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  // Load layout and set as root
  cegui->GetSystemPtr ()->setGUISheet(winMgr->loadWindowLayout("mech.layout"));

  // Subscribe to the clicked event for the exit button
  //CEGUI::Window* btn = winMgr->getWindow("Demo7/Window1/Quit");
  // btn->subscribeEvent(CEGUI::PushButton::EventClicked,
  //   CEGUI::Event::Subscriber(&CEGUITest::OnExitButtonClicked, this));
}

bool AppMech::LoadMap (const char* path)
{
  // Set VFS current directory to the level we want to load.
  csRef<iVFS> VFS (csQueryRegistry<iVFS> (GetObjectRegistry ()));
  const char* smartPath;
  CS::Utility::SmartChDir (VFS, path, "world", &smartPath);

  csRef<iThreadedLoader> tloader (csQueryRegistry<iThreadedLoader> (GetObjectRegistry ()));
  csRef<iThreadReturn> ret = tloader->LoadMapFileWait (VFS->GetCwd(), smartPath, false, 0,
						       0, 0, KEEP_ALL, true);

  VFS->PopDir ();
  if(!ret->WasSuccessful())
  {
    ReportError ("Failing to load map!");
    return false;
  }

  return true;
}

bool AppMech::PostLoadMap ()
{
  // Initialize collision objects for all loaded objects.
  csColliderHelper::InitializeCollisionWrappers (cdsys, engine);

  engine->Prepare ();

  // Creates an accessor for configuration settings.
  csConfigAccess cfgAcc (GetObjectRegistry ());

  // Find the starting position in this level.
  csVector3 pos (0);
  int nbStartPositions = engine->GetCameraPositions ()->GetCount ();
  if (nbStartPositions > 0)
  {
    // There is a valid starting position defined in the level file.
    iCameraPosition* campos = engine->GetCameraPositions ()->Get (0);
    sector = engine->GetSectors ()->FindByName (campos->GetSector ());
    pos = campos->GetPosition ();
    csPrintf ("\nFound %d starting position%s, using the first one:\n"
          "name: %s, sector: %s\n",
          nbStartPositions, nbStartPositions > 1 ? "s" : "",
          campos->QueryObject ()->GetName (), campos->GetSector ());
  }
  else
  {
    // We didn't find a valid starting position,
    // so we default to going to the first sector at position (0,2,0).
    sector = engine->GetSectors ()->Get (0);
    pos = csVector3 (GetConfigVector (cfgAcc, "DefaultStartPos", "0,2,0"));
    ReportWarning (
          "Didn't find any starting position, using %s in sector '%s'",
          pos.Description ().GetData (), sector->QueryObject ()->GetName ());
  }
  if (!sector)
    return ReportError("Couldn't find a valid sector in PostLoadMap()!");

  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (sector);
  view->GetCamera ()->GetTransform ().SetOrigin (pos);

  // Initialize our collider actor.
  collider_actor.SetCollideSystem (cdsys);
  collider_actor.SetEngine (engine);
  // Read the values from config files, and use default values if not set.
  csVector3 legs (GetConfigVector (cfgAcc, "Actor.Legs", "0.2,0.3,0.2"));
  csVector3 body (GetConfigVector (cfgAcc, "Actor.Body", "0.2,1.2,0.2"));
  csVector3 shift (GetConfigVector (cfgAcc, "Actor.Shift", "0.0,-1.0,0.0"));

  collider_actor.InitializeColliders (view->GetCamera (),
        legs, body, shift);

  // The static lighter provides a simple way to initialize the "static color"
  // of a mesh (usually genmesh) to provide a way to do simple static 'lighting'
  // Once you've lit your map using lighter2, disable it.
  // You must also disable it if you want to use hardware dynamic lighting,
  // like with parallax/normalmapping shaders.
  if (cfgAcc->GetBool ("StaticLighter.Enabled", true))
  {
    int maxLights = cfgAcc->GetInt ("StaticLighter.MaxLights", 4);
    using namespace CS::Lighting;
    SimpleStaticLighter::ShadowType shadowType =
      (SimpleStaticLighter::ShadowType)cfgAcc->GetInt (
      "StaticLighter.ShadowType", 0);
    // Note you could only "shine lights" on a specific mesh (see the API).
    SimpleStaticLighter::ShineLights (sector, engine, maxLights, shadowType);
  }

  return true;
}

bool AppMech::LoadLibrary (const char* path)
{
  csRef<iVFS> vfs (csQueryRegistry<iVFS> (GetObjectRegistry ()));
  if (vfs)
  {
    // Set current VFS dir to the level dir, helps with relative paths in maps
    vfs->PushDir (path);
    if (!loader->LoadLibraryFile (path))
    {
      vfs->PopDir ();
      return ReportError("Couldn't load library file %s!", path);
    }
    vfs->PopDir ();
  }
  else
    return ReportError ("Couldn't find VFS plugin!");

  return true;
}

