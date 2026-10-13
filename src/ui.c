#define UI_C_IMPLEMENTATION
#include "ui.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

Font g_ui_font = {0};
Font g_ui_font_bold = {0};
static bool s_fonts_loaded = false;

void ui_init_fonts(void) {
    if (s_fonts_loaded) return;

    /* Load modern TrueType font with mipmaps for crystal-clear typography */
    const char *font_candidates[] = {
        "assets/segoeui.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf"
    };

    for (size_t i = 0; i < sizeof(font_candidates)/sizeof(font_candidates[0]); i++) {
        if (FileExists(font_candidates[i])) {
            /* Regular UI Font: 28px base size (perfect for 10-22px UI text) */
            g_ui_font = LoadFontEx(font_candidates[i], 28, NULL, 0);
            if (IsFontReady(g_ui_font)) {
                GenTextureMipmaps(&g_ui_font.texture);
                SetTextureFilter(g_ui_font.texture, TEXTURE_FILTER_BILINEAR);
                s_fonts_loaded = true;
            }

            /* Title/Header Font: 44px base size for headers and metric cards */
            g_ui_font_bold = LoadFontEx(font_candidates[i], 44, NULL, 0);
            if (IsFontReady(g_ui_font_bold)) {
                GenTextureMipmaps(&g_ui_font_bold.texture);
                SetTextureFilter(g_ui_font_bold.texture, TEXTURE_FILTER_BILINEAR);
            }

            if (s_fonts_loaded) break;
        }
    }
}

void ui_close_fonts(void) {
    if (s_fonts_loaded) {
        if (IsFontReady(g_ui_font)) UnloadFont(g_ui_font);
        if (IsFontReady(g_ui_font_bold)) UnloadFont(g_ui_font_bold);
        s_fonts_loaded = false;
    }
}

void ui_draw_text(const char *text, int posX, int posY, int fontSize, Color color) {
    if (!text || strlen(text) == 0) return;
    if (s_fonts_loaded) {
        Font font_to_use = (fontSize >= 24 && IsFontReady(g_ui_font_bold)) ? g_ui_font_bold : g_ui_font;
        if (IsFontReady(font_to_use)) {
            DrawTextEx(font_to_use, text, (Vector2){ (float)posX, (float)posY }, (float)fontSize, 0.2f, color);
            return;
        }
    }
    DrawText(text, posX, posY, fontSize, color);
}

int ui_measure_text(const char *text, int fontSize) {
    if (!text || strlen(text) == 0) return 0;
    if (s_fonts_loaded) {
        Font font_to_use = (fontSize >= 24 && IsFontReady(g_ui_font_bold)) ? g_ui_font_bold : g_ui_font;
        if (IsFontReady(font_to_use)) {
            return (int)MeasureTextEx(font_to_use, text, (float)fontSize, 0.2f).x;
        }
    }
    return MeasureText(text, fontSize);
}

void ui_init(void) {
    ui_init_fonts();
}

void ui_draw_logo(Vector2 position, float scale, bool show_text, bool show_tagline) {
    float x = position.x;
    float y = position.y;
    float s = scale;

    /* Upwise Geometric Growth Emblem: Sleek modern upward dual chevrons */
    Vector2 v1 = { x + 6 * s,  y + 26 * s };
    Vector2 v2 = { x + 20 * s, y + 14 * s };
    Vector2 v3 = { x + 34 * s, y + 26 * s };
    DrawTriangle(v1, v2, (Vector2){ x + 20 * s, y + 20 * s }, COLOR_ACCENT);
    DrawTriangle(v2, v3, (Vector2){ x + 20 * s, y + 20 * s }, COLOR_ACCENT);

    /* Upper Primary Blue Chevron */
    Vector2 u1 = { x + 6 * s,  y + 16 * s };
    Vector2 u2 = { x + 20 * s, y + 4 * s };
    Vector2 u3 = { x + 34 * s, y + 16 * s };
    DrawTriangle(u1, u2, (Vector2){ x + 20 * s, y + 10 * s }, COLOR_PRIMARY);
    DrawTriangle(u2, u3, (Vector2){ x + 20 * s, y + 10 * s }, COLOR_PRIMARY);

    /* Central progress core dot */
    DrawCircleV((Vector2){ x + 20 * s, y + 10 * s }, 2.5f * s, (Color){ 255, 255, 255, 230 });

    if (show_text) {
        int font_size = (int)(26 * s);
        ui_draw_text("Upwise", (int)(x + 44 * s), (int)(y + 2 * s), font_size, COLOR_TEXT_PRIMARY);

        if (show_tagline) {
            int sub_size = (int)(11 * s);
            ui_draw_text("Learn. Practise. Progress.", (int)(x + 46 * s), (int)(y + 28 * s), sub_size, COLOR_TEXT_MUTED);
        }
    }
}

void ui_card(Rectangle bounds, Color bg_color, Color border_color, float radius) {
    DrawRectangleRounded(bounds, radius, 8, bg_color);
    DrawRectangleRoundedLines(bounds, radius, 8, 1.5f, border_color);
}

bool ui_button(Rectangle bounds, const char *label, ButtonStyle style, bool enabled) {
    Vector2 mouse_pos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse_pos, bounds) && enabled;
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    Color bg_color;
    Color border_color = COLOR_BORDER;
    Color text_color = COLOR_TEXT_PRIMARY;

    if (!enabled) {
        bg_color = (Color){ 20, 28, 44, 180 };
        border_color = (Color){ 30, 40, 60, 180 };
        text_color = COLOR_TEXT_DARK;
    } else {
        switch (style) {
            case BTN_PRIMARY:
                bg_color = hovered ? COLOR_PRIMARY_HOVER : COLOR_PRIMARY;
                border_color = bg_color;
                text_color = (Color){ 10, 15, 30, 255 };
                break;
            case BTN_SECONDARY:
                bg_color = hovered ? COLOR_CARD_HOVER : COLOR_CARD_BG;
                border_color = hovered ? COLOR_PRIMARY : COLOR_BORDER;
                text_color = hovered ? COLOR_PRIMARY : COLOR_TEXT_PRIMARY;
                break;
            case BTN_SUCCESS:
                bg_color = hovered ? (Color){ 65, 230, 170, 255 } : COLOR_SUCCESS;
                border_color = bg_color;
                text_color = (Color){ 8, 25, 18, 255 };
                break;
            case BTN_DANGER:
                bg_color = hovered ? (Color){ 255, 130, 150, 255 } : COLOR_ERROR;
                border_color = bg_color;
                text_color = (Color){ 35, 10, 15, 255 };
                break;
            case BTN_OUTLINE:
                bg_color = hovered ? (Color){ 91, 140, 255, 35 } : (Color){ 0, 0, 0, 0 };
                border_color = hovered ? COLOR_PRIMARY : COLOR_BORDER;
                text_color = hovered ? COLOR_PRIMARY : COLOR_TEXT_MUTED;
                break;
        }
    }

    DrawRectangleRounded(bounds, 0.25f, 6, bg_color);
    DrawRectangleRoundedLines(bounds, 0.25f, 6, 1.2f, border_color);

    /* Center text */
    int font_size = bounds.height > 40 ? 17 : 14;
    int text_width = ui_measure_text(label, font_size);
    int text_x = (int)(bounds.x + (bounds.width - text_width) / 2);
    int text_y = (int)(bounds.y + (bounds.height - font_size) / 2);

    ui_draw_text(label, text_x, text_y, font_size, text_color);

    return clicked;
}

bool ui_text_input(Rectangle bounds, TextInput *input, const char *placeholder) {
    if (!input) return false;

    Vector2 mouse_pos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse_pos, bounds);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        input->is_active = hovered;
    }

    if (input->is_active) {
        /* Process typing */
        int key = GetCharPressed();
        while (key > 0) {
            size_t len = strlen(input->text);
            if (key >= 32 && key <= 125 && len < (size_t)input->max_len && len < sizeof(input->text) - 1) {
                input->text[len] = (char)key;
                input->text[len + 1] = '\0';
                input->cursor_pos = (int)(len + 1);
            }
            key = GetCharPressed();
        }

        /* Backspace handling */
        if (IsKeyPressed(KEY_BACKSPACE) || (IsKeyDown(KEY_BACKSPACE) && GetTime() - input->blink_timer > 0.4f)) {
            size_t len = strlen(input->text);
            if (len > 0) {
                input->text[len - 1] = '\0';
                input->cursor_pos = (int)(len - 1);
            }
        }
    }

    Color bg_color = COLOR_CARD_BG;
    Color border_color = input->is_active ? COLOR_PRIMARY : (hovered ? COLOR_BORDER_ACTIVE : COLOR_BORDER);

    DrawRectangleRounded(bounds, 0.22f, 6, bg_color);
    DrawRectangleRoundedLines(bounds, 0.22f, 6, 1.5f, border_color);

    int font_size = 16;
    int text_y = (int)(bounds.y + (bounds.height - font_size) / 2);
    int text_x = (int)(bounds.x + 14);

    if (strlen(input->text) == 0) {
        ui_draw_text(placeholder ? placeholder : "", text_x, text_y, font_size, COLOR_TEXT_DARK);
    } else {
        ui_draw_text(input->text, text_x, text_y, font_size, COLOR_TEXT_PRIMARY);
    }

    /* Draw blinking cursor */
    if (input->is_active) {
        float time = (float)GetTime();
        if (fmodf(time, 1.0f) < 0.5f) {
            int cursor_x = text_x + ui_measure_text(input->text, font_size) + 2;
            DrawRectangle(cursor_x, text_y - 2, 2, font_size + 4, COLOR_PRIMARY);
        }
    }

    return input->is_active;
}

void ui_progress_bar(Rectangle bounds, float progress_fraction, Color fill_color, Color bg_color) {
    if (progress_fraction < 0.0f) progress_fraction = 0.0f;
    if (progress_fraction > 1.0f) progress_fraction = 1.0f;

    DrawRectangleRounded(bounds, 0.5f, 6, bg_color);

    if (progress_fraction > 0.01f) {
        Rectangle fill_rec = {
            bounds.x,
            bounds.y,
            bounds.width * progress_fraction,
            bounds.height
        };
        DrawRectangleRounded(fill_rec, 0.5f, 6, fill_color);
    }
}

void ui_badge(Vector2 position, const char *text, Color bg_color, Color text_color) {
    int font_size = 12;
    int text_w = ui_measure_text(text, font_size);
    Rectangle rec = { position.x, position.y, (float)(text_w + 16), 22.0f };

    DrawRectangleRounded(rec, 0.4f, 4, bg_color);
    ui_draw_text(text, (int)(position.x + 8), (int)(position.y + 3), font_size, text_color);
}

void ui_tooltip(Vector2 mouse_pos, const char *text) {
    int font_size = 12;
    int text_w = ui_measure_text(text, font_size);
    Rectangle rec = { mouse_pos.x + 12, mouse_pos.y + 12, (float)(text_w + 16), 24.0f };

    DrawRectangleRounded(rec, 0.3f, 4, (Color){ 15, 23, 42, 240 });
    DrawRectangleRoundedLines(rec, 0.3f, 4, 1.0f, COLOR_BORDER);
    ui_draw_text(text, (int)(rec.x + 8), (int)(rec.y + 4), font_size, COLOR_TEXT_PRIMARY);
}

void ui_draw_stat_card(Rectangle bounds, const char *title, const char *value, const char *subtitle, Color accent) {
    Vector2 mouse_pos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse_pos, bounds);
    Color bg = hovered ? COLOR_CARD_HOVER : COLOR_CARD_BG;
    Color border = hovered ? COLOR_BORDER_ACTIVE : COLOR_BORDER;

    ui_card(bounds, bg, border, 0.16f);

    /* Left accent vertical pill */
    Rectangle pill = { bounds.x + 4, bounds.y + bounds.height * 0.25f, 4, bounds.height * 0.5f };
    DrawRectangleRounded(pill, 0.5f, 4, accent);

    /* Title */
    ui_draw_text(title, (int)(bounds.x + 20), (int)(bounds.y + 14), 13, COLOR_TEXT_MUTED);

    /* Big Metric Value */
    ui_draw_text(value, (int)(bounds.x + 20), (int)(bounds.y + 32), 26, COLOR_TEXT_PRIMARY);

    /* Subtitle / Context */
    if (subtitle && strlen(subtitle) > 0) {
        ui_draw_text(subtitle, (int)(bounds.x + 20), (int)(bounds.y + bounds.height - 22), 12, COLOR_TEXT_DARK);
    }
}

void ui_draw_score_barchart(Rectangle bounds, const int scores[], int count, int max_score, const char *labels[]) {
    ui_card(bounds, COLOR_CARD_BG, COLOR_BORDER, 0.14f);

    int title_size = 15;
    ui_draw_text("Recent Quiz Score Performance (%)", (int)(bounds.x + 20), (int)(bounds.y + 16), title_size, COLOR_TEXT_PRIMARY);

    if (count <= 0) {
        ui_draw_text("No quiz attempts recorded yet. Take a quiz to populate performance charts.",
                     (int)(bounds.x + 20), (int)(bounds.y + bounds.height / 2 - 8), 13, COLOR_TEXT_MUTED);
        return;
    }

    float chart_x = bounds.x + 45;
    float chart_y = bounds.y + 48;
    float chart_w = bounds.width - 65;
    float chart_h = bounds.height - 80;

    /* Draw horizontal grid lines for 25%, 50%, 75%, 100% */
    int grid_steps[4] = {25, 50, 75, 100};
    for (int g = 0; g < 4; g++) {
        float y_pos = chart_y + chart_h - (chart_h * (grid_steps[g] / 100.0f));
        DrawLine((int)chart_x, (int)y_pos, (int)(chart_x + chart_w), (int)y_pos, (Color){ 35, 48, 75, 120 });
        char num[8];
        snprintf(num, sizeof(num), "%d%%", grid_steps[g]);
        ui_draw_text(num, (int)(bounds.x + 10), (int)(y_pos - 6), 10, COLOR_TEXT_DARK);
    }

    /* Baseline */
    DrawLine((int)chart_x, (int)(chart_y + chart_h), (int)(chart_x + chart_w), (int)(chart_y + chart_h), COLOR_BORDER);

    /* Draw bars */
    float slot_width = chart_w / (float)count;
    float bar_width = slot_width * 0.55f;
    if (bar_width > 42.0f) bar_width = 42.0f;

    Vector2 mouse_pos = GetMousePosition();

    for (int i = 0; i < count; i++) {
        int score = scores[i];
        if (score < 0) score = 0;
        if (score > max_score) score = max_score;

        float bar_h = (chart_h * score) / (float)max_score;
        float bar_x = chart_x + (i * slot_width) + (slot_width - bar_width) / 2.0f;
        float bar_y = chart_y + chart_h - bar_h;

        Rectangle bar_rec = { bar_x, bar_y, bar_width, bar_h };

        Color bar_col = (score >= 80) ? COLOR_SUCCESS : ((score >= 50) ? COLOR_WARNING : COLOR_ERROR);

        bool bar_hovered = CheckCollisionPointRec(mouse_pos, bar_rec);
        if (bar_hovered) {
            bar_col = (Color){ (unsigned char)fmin(bar_col.r + 30, 255),
                               (unsigned char)fmin(bar_col.g + 30, 255),
                               (unsigned char)fmin(bar_col.b + 30, 255), 255 };
        }

        if (bar_h > 2.0f) {
            DrawRectangleRounded(bar_rec, 0.3f, 4, bar_col);
        }

        /* Label at bottom */
        if (labels && labels[i]) {
            int lbl_w = ui_measure_text(labels[i], 11);
            ui_draw_text(labels[i], (int)(bar_x + (bar_width - lbl_w) / 2), (int)(chart_y + chart_h + 8), 11, COLOR_TEXT_MUTED);
        }

        /* Percentage on top of bar */
        char val_str[8];
        snprintf(val_str, sizeof(val_str), "%d%%", score);
        int val_w = ui_measure_text(val_str, 10);
        ui_draw_text(val_str, (int)(bar_x + (bar_width - val_w) / 2), (int)(bar_y - 14), 10, COLOR_TEXT_PRIMARY);

        if (bar_hovered) {
            char tip[64];
            snprintf(tip, sizeof(tip), "Attempt %d: %d%% score", i + 1, score);
            ui_tooltip(mouse_pos, tip);
        }
    }
}
