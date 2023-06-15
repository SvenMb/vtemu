// #include "Stream.h"

#include <Arduino_GFX_Library.h>
#include <Print.h>


class Terminal {
 
    public:

        bool begin();

        bool begin(Arduino_GFX * gfx, int maxCol = -1, int maxRow = -1);

        Arduino_Canvas_Indexed * getCanvas();

        void write(int c);

};