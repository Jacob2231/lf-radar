#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>

typedef struct {
    bool running;
    uint8_t signal;
    char status[32];
    FuriMutex* mutex;
} App;

static void draw(Canvas* canvas, void* ctx) {
    App* app = ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "LF Radar (Momentum)");

    canvas_set_font(canvas, FontSecondary);

    if(app->running) {
        canvas_draw_str(canvas, 2, 25, "Scanning LF field...");

        // bars
        uint8_t bars = app->signal / 10;
        for(uint8_t i = 0; i < bars; i++) {
            canvas_draw_box(canvas, 2 + i * 5, 40, 3, 10);
        }

        char buf[64];
        snprintf(buf, sizeof(buf), "Signal: %d%%", app->signal);
        canvas_draw_str(canvas, 2, 60, buf);

        canvas_draw_str(canvas, 2, 75, app->status);

    } else {
        canvas_draw_str(canvas, 2, 30, "Press OK to start");
    }

    furi_mutex_release(app->mutex);
}

static void input(InputEvent* event, void* ctx) {
    App* app = ctx;

    if(event->type == InputTypePress && event->key == InputKeyOk) {
        furi_mutex_acquire(app->mutex, FuriWaitForever);
        app->running = !app->running;
        furi_mutex_release(app->mutex);
    }
}

// псевдо "поле"
static void loop(void* p) {
    App* app = p;

    while(1) {
        if(!app->running) {
            furi_delay_ms(100);
            continue;
        }

        // имитация "шума поля"
        int base = rand() % 30;

        // редкие пики (как будто метка рядом)
        if(rand() % 20 == 0) {
            base += 60;
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            snprintf(app->status, sizeof(app->status), "TAG DETECTED");
            furi_mutex_release(app->mutex);

            furi_hal_vibro_on(true);
            furi_delay_ms(50);
            furi_hal_vibro_on(false);

            furi_hal_speaker_start(1200, 0.3);
            furi_delay_ms(50);
            furi_hal_speaker_stop();
        } else {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            snprintf(app->status, sizeof(app->status), "Searching...");
            furi_mutex_release(app->mutex);
        }

        if(base > 100) base = 100;

        furi_mutex_acquire(app->mutex, FuriWaitForever);
        app->signal = base;
        furi_mutex_release(app->mutex);

        furi_delay_ms(120);
    }
}

int32_t lf_radar_app(void* p) {
    UNUSED(p);

    App* app = malloc(sizeof(App));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->running = false;
    app->signal = 0;
    snprintf(app->status, sizeof(app->status), "Idle");

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* vp = view_port_alloc();

    view_port_draw_callback_set(vp, draw, app);
    view_port_input_callback_set(vp, input, app);

    gui_add_view_port(gui, vp, GuiLayerFullscreen);

    FuriThread* th = furi_thread_alloc();
    furi_thread_set_name(th, "lf_radar");
    furi_thread_set_context(th, app);
    furi_thread_set_callback(th, loop);
    furi_thread_start(th);

    while(1) {
        furi_delay_ms(200);
    }

    return 0;
}
