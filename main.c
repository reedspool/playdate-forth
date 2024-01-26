//
//  main.c
//
//  Created by Reed Spool and Quinten Konyn!
//

#include "pd_api.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static PlaydateAPI *pd = NULL;
static int PX_WIDTH = 400;
static int PX_HEIGHT = 280;

static int update(void *ud) { return 1; }

#ifdef _WINDLL
__declspec(dllexport)
#endif
    int eventHandler(PlaydateAPI *playdate, PDSystemEvent event, uint32_t arg) {
  if (event == kEventInit) {
    pd = playdate;
    pd->display->setRefreshRate(0); // run as fast as possible
    pd->system->setUpdateCallback(update, NULL);
  }

  return 0;
}
