/*
    I try to be ctrl-code compatible to fabgl::terminal
*/

#include <Arduino_GFX_Library.h>
#include <Print.h>
#include <elapsedMillis.h>

class Terminal : public Print {

    Arduino_Canvas_Indexed * _canvas = NULL;
    Arduino_GFX * _gfx = NULL;

    uint8_t * _framebuffer = NULL;
    uint16_t * _col_index = NULL;

    int _max_x; // pixel x used by vtemu for chars
    int _max_y; // pixel y used by vtemu for chars 
    int _off_x; // offset on gfx
    int _off_y;
    int _cols;  // char columns
    int _rows;  // char rows
    uint16_t _font_height = 8; 
    uint16_t _font_width = 6;
    int _cursor_old_x = 1;
    int _cursor_old_y = 1;
    
    elapsedMillis  _cursor_millis = 0;
    // 0 block, 1 block off, 2 underline but on, 3 underline off, 4,5 bar not implemented now
    // -1 no cursor
    int8_t _cursor_state = 0;

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

            _canvas->setCursor(0, 0);
            _canvas->setTextColor(GREEN,BLACK);
            _canvas->printf(" VTemu 0.1\r\n===========\r\nsven@muehlberg.net\r\n\r\n");
            _canvas->printf("%dx%d chars in %dx%d pixel\r\n\r\n",_cols, _rows,_max_x,_max_y);
            _canvas->flush();

            _gfx->setCursor(_canvas->getCursorX()+_off_x,_canvas->getCursorY()+_off_y);
            _gfx->setTextColor(GREEN,BLACK);
    
            _framebuffer = _canvas->getFramebuffer();
            _col_index = _canvas->getColorIndex();

            return true;
        }

        void scroll_up() {
            uint8_t *framebuffer = _canvas->getFramebuffer();
            // smooth scroll
            for(int i = 0; i < _font_height; ++i) {
                memmove(framebuffer,framebuffer+_max_x,(_max_y-1)*_max_x);
                _canvas->drawFastHLine(0,_max_y-1,_max_x,BLACK);
                _canvas->flush();
            }

            return;
        }

        void undrawCursor() {
            _cursor_millis = 0;
            _cursor_state = _cursor_state & 0x0e;
            // Serial.printf("CS0: %d\n",_cursor_state);
             
            for(int i = 0; i < _font_height;i++) {
                _gfx->drawIndexedBitmap(_cursor_old_x+_off_x,_cursor_old_y+_off_y + i,
                                        &_framebuffer[ _max_x * ( _cursor_old_x + i ) + _cursor_old_y],
                                        _col_index,_font_width,1 );
            }
            return;
        };

        void drawCursor() {
            uint8_t *framebuffer = _canvas->getFramebuffer();

            _cursor_old_x = _canvas->getCursorX();
            _cursor_old_y = _canvas->getCursorY();
            _cursor_millis = 0;
            _cursor_state = _cursor_state | 0x01;
            // Serial.printf("CS1: %d\n",_cursor_state);

            for(int i = 0; i < _font_height;i++) {
                _gfx->drawFastHLine(_cursor_old_x + _off_x,_cursor_old_y + _off_y + i,_font_width,YELLOW);
            }

            return;
        };

        void blinkCursor() {

            if (_cursor_millis > 500) {
                if (_cursor_state & 1) {
                    undrawCursor();
                } else {
                    drawCursor();
                }
            }
        };

        size_t write(uint8_t c) {
            // normal char or part of unicode
            if ( c>=0x20 && c<0x7f )  {
                _canvas->write(c);
                _gfx->write(c);
                if (_canvas->getCursorY() >= _rows * _font_height  ||(_canvas->getCursorY()>= (_rows-1) *_font_height && _canvas->getCursorX() >= _cols * _font_width)) {
                    scroll_up();
                    _canvas->setCursor(0,_canvas->getCursorY());
                    _gfx->setCursor(_off_x,_gfx->getCursorY());
                } else if (_canvas->getCursorX() == _max_x) {
                    _canvas->setCursor(0,_canvas->getCursorY()+_font_height);
                    _gfx->setCursor(_off_x,_off_y+_canvas->getCursorY());
                }
            // backspace
            } else if (c == 0x7f) { // backspace
            } else if (c == 0x0a) { // line feed
                undrawCursor();
                if (_canvas->getCursorY() > 19*_font_height) {
                    scroll_up();
                } else {
                    _canvas->setCursor(_canvas->getCursorX(),_canvas->getCursorY()+_font_height);
                    _gfx->setCursor(_gfx->getCursorX(),_gfx->getCursorY()+_font_height);
                }
            } else if (c == 0x0d) { // carriage return
                undrawCursor();
                _canvas->setCursor(0,_canvas->getCursorY());
                _gfx->setCursor(_off_x,_gfx->getCursorY());
                return 1;
            } else if (c == 0x0c) { // clear screen, should be same as line feed?
                _canvas->fillScreen(BLACK);
                _canvas->setCursor(0,0);
                _gfx->setCursor(_off_x,_off_y);
                _canvas->flush();
            }

            drawCursor();
            //_canvas->flush();   
            return 1;
        };
};
