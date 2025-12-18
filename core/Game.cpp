class Game {
private:
    Grid grid;
    bool gameOver;

public:
    Game();
    void start();
    void update(Direction dir);
    bool isGameOver() const;
    const Grid& getGrid() const;
};
