/*
    I try to be ctrl-code compatible to fabgl::terminal
*/

#include <Arduino_GFX_Library.h>
#include <Print.h>

class Terminal : public Print {

    Arduino_Canvas_Indexed * _canvas = NULL;
    Arduino_GFX * _gfx = NULL;
    int _max_x; // pixel x used by vtemu for chars
    int _max_y; // pixel y used by vtemu for chars 
    int _off_x; // offset on gfx
    int _off_y;
    int _cols;  // char rows
    int _rows;  // char columns
    int _font_height = 8; 
    int _font_width = 6;
    int _cursor = 0;

    public:
        Terminal() {
        }

        Terminal(Arduino_GFX * gfx, int max_x, int max_y, int offset_x, int offset_y) {

            begin(gfx,max_x,max_y,offset_x,offset_y);
        }

        ~Terminal() {
            if (_canvas!=NULL) {
                delete _canvas;
            }
        }

        bool begin(Arduino_GFX * gfx, int max_x, int max_y,int offset_x,int offset_y) {

            _gfx = gfx;
            _cols = max_x/_font_width;
            _rows = max_y/_font_height;
            _max_x = _cols * _font_width;
            _max_y = _rows * _font_height;
            _off_x = offset_x;
            _off_y = offset_y;

            begin();

            return true;
        }

        bool begin() {

            if (_canvas == NULL) { // only allocate and begin the canvas when called first time
                _canvas = new Arduino_Canvas_Indexed(_max_x /* width */, _max_y /* height */, _gfx,_off_x,_off_y);
                _canvas->begin();
            }
            _canvas->fillScreen(BLACK);
            // vt_canvas->flush();

            _canvas->setCursor(0, 0);
            _canvas->setTextColor(GREEN,BLACK);
            _canvas->printf(" VTemu 0.1\r\n===========\r\nsven@muehlberg.net\r\n\r\n");
            _canvas->printf("%dx%d chars in %dx%d pixel\r\n\r\n",_cols, _rows,_max_x,_max_y);
            _canvas->flush();
            _gfx->setCursor(_canvas->getCursorX()+_off_x,_canvas->getCursorY()+_off_y);
            _gfx->setTextColor(GREEN,BLACK);            

            return true;
        }

        // Arduino_Canvas_Indexed * getCanvas() {
        //    return _canvas;
        // }

        void scroll_up() {
            uint8_t *framebuffer = _canvas->getFramebuffer();
            for(int i = 0; i < _font_height; ++i) { // smooth scroll
                memmove(framebuffer,framebuffer+_max_x,(_max_y-1)*_max_x);
                _canvas->drawFastHLine(0,_max_y-1,_max_x,BLACK);
                _canvas->flush();
            }
        }

        size_t write(uint8_t c) {
            if (c>=0x20 && c<0x7f) {
                _canvas->write(c);
                _gfx->write(c);
                if (_canvas->getCursorY() >= _rows * _font_height  ||(_canvas->getCursorY()>= (_rows-1) *_font_height && _canvas->getCursorX() >= _cols * _font_width)) {
                    scroll_up();
                    _canvas->setCursor(0,_canvas->getCursorY());
                    _gfx->setCursor(_canvas->getCursorX()+_off_x,_canvas->getCursorY()+_off_y);
                }
            } else if (c == 0x0a) { // line feed
                if (_canvas->getCursorY() > 19*_font_height) {
                    scroll_up();
                } else {
                    _canvas->setCursor(_canvas->getCursorX(),_canvas->getCursorY()+_font_height);
                    _gfx->setCursor(_canvas->getCursorX()+_off_x,_canvas->getCursorY()+_off_y);
                }
            } else if (c == 0x0d) { // carriage return
                _canvas->setCursor(0,_canvas->getCursorY());
                _gfx->setCursor(_canvas->getCursorX()+_off_x,_canvas->getCursorY()+_off_y);
            } else if (c == 0x0c) { // clear screen
                _canvas->fillScreen(BLACK);
                _canvas->setCursor(0,0);
                _canvas->flush();
                _gfx->setCursor(_canvas->getCursorX()+_off_x,_canvas->getCursorY()+_off_y);
            }
            return 1;
        };
};
