/*
 * appmech.h
 *
 * Declaration of AppMech, the main application object.
 */

#ifndef __moveController_h
#define __moveController_h

#include "mech.h"

class MoveController
{
private:

  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------

  void calculateMovement(csRef<iKeyboardDriver> kdb);

  // ---------------------------------------------------------------------------

public:
  /**
   * Constructor.
   */
  /* MoveController (); */

  /**
   * Destructor.
   */
  /* virtual ~MoveController(); */
};

#endif // __moveController_h

