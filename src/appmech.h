/*
 * appmech.h
 *
 * Declaration of AppMech, the main application object.
 */

#ifndef __appmech_h
#define __appmech_h

#include <crystalspace.h>
#include <ivaria/icegui.h>

#include <physicallayer/entity.h>
#include <physicallayer/pl.h>
#include <behaviourlayer/bl.h>

#include "moveController.h"

class AppMech :
  public csApplicationFramework, public csBaseEventHandler
{
private:

  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iVirtualClock> vc;
  csRef<iCollideSystem> cdsys;
  csRef<iView> view;
  csRef<FramePrinter> printer;
  csRef<iConfigManager> cfgmgr;
  csRef<iCEGUI> cegui;

  csRef<iCelPlLayer> pl;
  csRef<iCelEntity> game;
  csRef<iCelEntity> entity_cam;

  /// A pointer to the sector the camera will be in.
  iSector* sector;

  /// Our collider used for gravity and CD (collision detection).
  csColliderActor collider_actor;

  csVector3 movement;
  float speed;
  float gyro;

  // ---------------------------------------------------------------------------

  MoveController move_controller;

  void SetupCegui ();
  void SetupWorld ();

  void CaptureInputState ();

  // ---------------------------------------------------------------------------

  /// Load the 'world' file in the given VFS map directory.
  bool LoadMap (const char* path);

  /// Setup stuff after map loading.
  bool PostLoadMap ();

  /// Load a library file with the given VFS path.
  bool LoadLibrary (const char* path);

  /**
   * Set up everything that needs to be rendered on screen.  This routine is
   * called from the event handler in response to a csevFrame event.
   */
  virtual void Frame();

  /**
   * Handle keyboard events, such as key presses and releases.  This routine is
   * called from the event handler in response to a csevKeyboard event.
   */
  virtual bool OnKeyboard(iEvent&);

  // ---------------------------------------------------------------------------

public:
  /**
   * Constructor.
   */
  AppMech();

  /**
   * Destructor.
   */
  virtual ~AppMech();

  /**
   * Final cleanup.
   */
  virtual void OnExit();

  /**
   * Main initialization routine.  This routine should set up basic facilities
   * (such as loading startup-time plugins, etc.).  In case of failure this
   * routine will return false.  You can assume that the error message has been
   * reported to the user.
   */
  virtual bool OnInitialize(int argc, char* argv[]);

  /**
   * Run the application.  Performs additional initialization (if needed), and
   * then fires up the main run/event loop.  The loop will fire events which
   * actually causes Crystal Space to "run".  Only when the program exits does
   * this function return.
   */
  virtual bool Application();

  /* Declare the name by which this class is identified to the event scheduler.
   * Declare that we want to receive the frame event in the "LOGIC" phase,
   * and that we're not terribly interested in having other events
   * delivered to us before or after other modules, plugins, etc. */
  CS_EVENTHANDLER_PHASE_LOGIC("application.mech")
};

#endif // __appmech_h
