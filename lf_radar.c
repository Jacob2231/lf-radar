#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>

typedef struct {
    bool running;
    uint8_t signal;
} App;

static void draw(Canvas* canvas, void* ctx) {
    App* app = ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "LF Radar");

    canvas_set_font(canvas, FontSecondary);

    if(app->running) {
        canvas_draw_str(canvas, 2, 25, "Scanning...");

        char buf[32];
        snprintf(buf, sizeof(buf), "Signal: %d%%", app->signal);
        canvas_draw_str(canvas, 2, 40, buf);
    } else {
        canvas_draw_str(canvas, 2, 25, "Press OK");
    }
}

static void input(InputEvent* event, void* ctx) {
    App* app = ctx;

    if(event->type == InputTypePress && event->key == InputKeyOk) {
        app->running = !app->running;
    }
}

int32_t lf_radar_app(void* p) {
    UNUSED(p);

    App app = {0};

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* vp = view_port_alloc();

    view_port_draw_callback_set(vp, draw, &app);
    view_port_input_callback_set(vp, input, &app);

    gui_add_view_port(gui, vp, GuiLayerFullscreen);

    while(1) {
        if(app.running) {
            app.signal = (app.signal + (rand() % 15)) % 100;
        }
        furi_delay_ms(150);
    }

    return 0;
}
