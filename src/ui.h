#ifndef UI_H
#define UI_H

#include <raylib.h>
#include <stdbool.h>

/* UI Color Palette (Section 2 Brand Identity) */
#define COLOR_BG_MAIN       (Color){ 11, 16, 32, 255 }     /* #0B1020 */
#define COLOR_BG_SECONDARY  (Color){ 17, 26, 46, 255 }     /* #111A2E */
#define COLOR_CARD_BG       (Color){ 23, 34, 56, 255 }     /* #172238 */
#define COLOR_CARD_HOVER    (Color){ 30, 45, 74, 255 }     /* #1E2D4A */
#define COLOR_BORDER        (Color){ 37, 53, 86, 255 }     /* #253556 */
#define COLOR_BORDER_ACTIVE (Color){ 91, 140, 255, 180 }
#define COLOR_PRIMARY       (Color){ 91, 140, 255, 255 }   /* #5B8CFF */
#define COLOR_PRIMARY_HOVER (Color){ 115, 158, 255, 255 }
#define COLOR_ACCENT        (Color){ 139, 92, 246, 255 }   /* #8B5CF6 */
#define COLOR_SUCCESS       (Color){ 52, 211, 153, 255 }   /* #34D399 */
#define COLOR_WARNING       (Color){ 251, 191, 36, 255 }   /* #FBBF24 */
#define COLOR_ERROR         (Color){ 251, 113, 133, 255 }  /* #FB7185 */
#define COLOR_TEXT_PRIMARY  (Color){ 241, 245, 249, 255 }  /* #F1F5F9 */
#define COLOR_TEXT_MUTED    (Color){ 148, 163, 184, 255 }  /* #94A3B8 */
#define COLOR_TEXT_DARK     (Color){ 100, 116, 139, 255 }  /* #64748B */

typedef enum {
    BTN_PRIMARY,
    BTN_SECONDARY,
    BTN_SUCCESS,
    BTN_DANGER,
    BTN_OUTLINE
} ButtonStyle;

typedef struct {
    char text[128];
    int cursor_pos;
    bool is_active;
    int max_len;
    float blink_timer;
} TextInput;

extern Font g_ui_font;
extern Font g_ui_font_bold;

/* High-DPI Font Management */
void ui_init_fonts(void);
void ui_close_fonts(void);
void ui_draw_text(const char *text, int posX, int posY, int fontSize, Color color);
int ui_measure_text(const char *text, int fontSize);

#ifndef UI_C_IMPLEMENTATION
#define DrawText(text, x, y, size, ...) ui_draw_text((text), (x), (y), (size), __VA_ARGS__)
#define MeasureText(text, size) ui_measure_text((text), (size))
#endif

/* UI Drawing & Interaction Primitives */
void ui_init(void);
void ui_draw_logo(Vector2 position, float scale, bool show_text, bool show_tagline);
bool ui_button(Rectangle bounds, const char *label, ButtonStyle style, bool enabled);
void ui_card(Rectangle bounds, Color bg_color, Color border_color, float radius);
bool ui_text_input(Rectangle bounds, TextInput *input, const char *placeholder);
void ui_progress_bar(Rectangle bounds, float progress_fraction, Color fill_color, Color bg_color);
void ui_badge(Vector2 position, const char *text, Color bg_color, Color text_color);
void ui_tooltip(Vector2 mouse_pos, const char *text);

/* Data Visualizations */
void ui_draw_stat_card(Rectangle bounds, const char *title, const char *value, const char *subtitle, Color accent);
void ui_draw_score_barchart(Rectangle bounds, const int scores[], int count, int max_score, const char *labels[]);

#endif /* UI_H */
