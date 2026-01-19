#include "TileView.hpp"
#include <sstream>
#include <cmath>

namespace {

// Retourne la couleur du texte en fonction de la valeur
SDL_Color textColorFor(int value) {
    if(value <= 8) return SDL_Color{80,40,60,255};
    return SDL_Color{255,255,255,235};
}

// Définit la couleur du renderer
void setColor(SDL_Renderer* r, SDL_Color c) { 
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a); 
}

} // namespace

void TileView::drawDigit(SDL_Renderer* r, int digit, int x, int y, int w, int h, SDL_Color color) const {
    // Segments pour chaque chiffre (7 segments)
    static const int segs[10] = {
        0b1111110, 0b0110000, 0b1101101, 0b1111001, 0b0110011,
        0b1011011, 0b1011111, 0b1110000, 0b1111111, 0b1111011
    };
    const int mask = (digit >= 0 && digit <= 9) ? segs[digit] : 0;
    setColor(r, color);

    const int t = std::max(2, w / 6);
    const int pad = std::max(2, t / 2);

    // Lambda pour obtenir SDL_Rect pour un segment
    auto segRect = [&](char seg) -> SDL_Rect {
        switch(seg) {
            case 'a': return SDL_Rect{x + pad, y, w - 2 * pad, t};
            case 'd': return SDL_Rect{x + pad, y + h - t, w - 2 * pad, t};
            case 'g': return SDL_Rect{x + pad, y + (h - t) / 2, w - 2 * pad, t};
            case 'f': return SDL_Rect{x, y + pad, t, (h - 3 * pad) / 2};
            case 'b': return SDL_Rect{x + w - t, y + pad, t, (h - 3 * pad) / 2};
            case 'e': return SDL_Rect{x, y + (h + pad) / 2, t, (h - 3 * pad) / 2};
            case 'c': return SDL_Rect{x + w - t, y + (h + pad) / 2, t, (h - 3 * pad) / 2};
            default: return SDL_Rect{x, y, 0, 0};
        }
    };

    // Lambda pour dessiner un segment si le bit correspondant est actif
    auto drawIf = [&](char seg, int bit){
        if(mask & bit) {
            SDL_Rect rect = segRect(seg); // stocker dans variable locale
            SDL_RenderFillRect(r, &rect); // passer l'adresse de la variable
        }
    };

    drawIf('a', 1 << 6);
    drawIf('b', 1 << 5);
    drawIf('c', 1 << 4);
    drawIf('d', 1 << 3);
    drawIf('e', 1 << 2);
    drawIf('f', 1 << 1);
    drawIf('g', 1 << 0);
}

void TileView::drawNumber(SDL_Renderer* r, const SDL_Rect& rect, int value) const {
    if(value <= 0) return;

    std::ostringstream ss; 
    ss << value;
    std::string s = ss.str();

    SDL_Color color = textColorFor(value);

    int maxDigits = static_cast<int>(s.size());
    int digitW = std::max(10, rect.w / std::max(2, maxDigits));
    int digitH = std::max(18, rect.h / 2);
    int totalW = digitW * maxDigits + (maxDigits - 1) * (digitW / 6);
    int x = rect.x + (rect.w - totalW) / 2;
    int y = rect.y + (rect.h - digitH) / 2;

    for(char ch : s) { 
        drawDigit(r, ch - '0', x, y, digitW, digitH, color); 
        x += digitW + (digitW / 6); 
    }
}
