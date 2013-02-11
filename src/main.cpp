/*
 * main.cpp
 *
 * Main entry point for mech.
 */

#include "mech.h"
#include "appmech.h"
#include <csutil/sysfunc.h> // Provides csPrintf()

CS_IMPLEMENT_APPLICATION

int main(int argc, char** argv)
{
  //csPrintf ("mech version 0.1 by Fossi.\n");

  /* Runs the application.  
   *
   * csApplicationRunner<> cares about creating an application instance 
   * which will perform initialization and event handling for the entire game. 
   *
   * The underlying csApplicationFramework also performs some core 
   * initialization.  It will set up the configuration manager, event queue, 
   * object registry, and much more.  The object registry is very important, 
   * and it is stored in your main application class (again, by 
   * csApplicationFramework). 
   */
  return csApplicationRunner<AppMech>::Run (argc, argv);
}
