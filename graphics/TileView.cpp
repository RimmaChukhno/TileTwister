class TileView : public GameObject {
private:
    int value;

public:
    TileView(int x, int y, int value);
    void render(SDL_Renderer* renderer) override;
};
