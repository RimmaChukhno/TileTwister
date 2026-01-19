#include "Score.hpp"

#include <algorithm>
#include <sstream>

namespace {

void setColor(SDL_Renderer* r, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}

SDL_Color textColorForScore() {
    return SDL_Color{255, 255, 255, 255};
}

void drawDigit(SDL_Renderer* r, int digit, int x, int y, int w, int h,
               SDL_Color color) {
    // Seven segments: a b c d e f g
    static const int segs[10] = {
        /*0*/ 0b1111110,
        /*1*/ 0b0110000,
        /*2*/ 0b1101101,
        /*3*/ 0b1111001,
        /*4*/ 0b0110011,
        /*5*/ 0b1011011,
        /*6*/ 0b1011111,
        /*7*/ 0b1110000,
        /*8*/ 0b1111111,
        /*9*/ 0b1111011,
    };

    const int mask = (digit >= 0 && digit <= 9) ? segs[digit] : 0;
    setColor(r, color);

    const int t = std::max(2, w / 6);   // épaisseur segments
    const int pad = std::max(2, t / 2);

    auto segRect = [&](char seg) -> SDL_Rect {
        switch (seg) {
        case 'a':
            return SDL_Rect{x + pad, y, w - 2 * pad, t};
        case 'd':
            return SDL_Rect{x + pad, y + h - t, w - 2 * pad, t};
        case 'g':
            return SDL_Rect{x + pad, y + (h - t) / 2, w - 2 * pad, t};
        case 'f':
            return SDL_Rect{x, y + pad, t, (h - 3 * pad) / 2};
        case 'b':
            return SDL_Rect{x + w - t, y + pad, t, (h - 3 * pad) / 2};
        case 'e':
            return SDL_Rect{x, y + (h + pad) / 2, t, (h - 3 * pad) / 2};
        case 'c':
            return SDL_Rect{x + w - t, y + (h + pad) / 2, t, (h - 3 * pad) / 2};
        default:
            return SDL_Rect{};
        }
    };

    // Correction: ne plus prendre l'adresse d'un temporaire
    auto drawSeg = [&](char s) {
        SDL_Rect rect = segRect(s);
        SDL_RenderFillRect(r, &rect);
    };

    if (mask & (1 << 0)) drawSeg('a');
    if (mask & (1 << 1)) drawSeg('b');
    if (mask & (1 << 2)) drawSeg('c');
    if (mask & (1 << 3)) drawSeg('d');
    if (mask & (1 << 4)) drawSeg('e');
    if (mask & (1 << 5)) drawSeg('f');
    if (mask & (1 << 6)) drawSeg('g');
}

void drawNumber(SDL_Renderer* r, const SDL_Rect& rect, int value) {
    if (value <= 0) return;

    std::ostringstream ss;
    ss << value;
    const std::string s = ss.str();

    const SDL_Color color = textColorForScore();

    const int maxDigits = static_cast<int>(s.size());
    const int digitW = std::max(10, rect.w / std::max(2, maxDigits));
    const int digitH = std::max(18, rect.h / 2);
    const int totalW = digitW * maxDigits + (maxDigits - 1) * (digitW / 6);

    int x = rect.x + (rect.w - totalW) / 2;
    const int y = rect.y + (rect.h - digitH) / 2;

    for (char ch : s) {
        const int d = ch - '0';
        drawDigit(r, d, x, y, digitW, digitH, color);
        x += digitW + (digitW / 6);
    }
}

} // namespace

void Score::render(SDL_Renderer* r, int x, int y, int w, int h) const {
    SDL_Rect rect = {x, y, w, h};
    drawNumber(r, rect, m_score);
}
