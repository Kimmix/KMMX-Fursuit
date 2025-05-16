#pragma once
#include <Arduino.h>

#include "accColor.h"
#include "Icons.h"

#if defined(USE_RLE_BITMAPS) && USE_RLE_BITMAPS
#include "mouth_RLE/mouthBitmaps.h"
#include "eyes_RLE/eyeBitmaps.h"
#else
#include "mouth/mouthBitmaps.h"
#include "eyes/eyeBitmaps.h"
#endif