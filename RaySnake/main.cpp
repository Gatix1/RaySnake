#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <deque>

const Color DARK_COLOR = { 63, 41, 30, 255 };
const Color LIGHT_COLOR = { 253, 202, 85, 255 };

const int cellSize = 30;
const int cellsWidth = 25;
const int cellsHeight = 40;
const int offset = 75;

const double updateTime = 0.1;

double lastUpdateTime = 0;

const int screenWidth = cellSize * cellsWidth + offset * 2;
const int screenHeight = cellSize * cellsHeight + offset * 2;

bool IsTimeElapsed(double interval) {
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval) {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

bool ElementInDeque(std::deque<Vector2> deque, Vector2 element) {
    for (unsigned int i = 0; i < deque.size(); i++) {
        if (Vector2Equals(deque[i], element)) {
            break;
        }
        else if (i == deque.size() - 1) {
            return false;
        }
    }
    return true;
}

class Food {
private:
    Vector2 position = {0, 0};
    Texture2D texture;
public:
    Food() {
        Image image = LoadImage("images/food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }
    ~Food() {
        UnloadTexture(texture);
    }

    void SetPosition(Vector2 newPosition) {
        position = newPosition;
    }

    Vector2 GetPosition() {
        return position;
    }

    Vector2 GenerateRandomPosition(std::deque<Vector2> snakeBody) {
        Vector2 result;
        bool resulted = false;

        // Resulted for making sure the food doesnt spawn in snake body
        while (!resulted) {
            result.x = GetRandomValue(0, cellsWidth - 1);
            result.y = GetRandomValue(0, cellsHeight - 1);

            resulted = !ElementInDeque(snakeBody, result);
        }
        return result;
    }

    void Draw() {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, LIGHT_COLOR);
    }
};

class Snake {
private:
    std::deque<Vector2> body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
    Vector2 direction = { 1, 0 };
    Vector2 prevDirection = direction;
    bool addingSegment = false;
public:
    void Update() {
        if (addingSegment) {
            body.push_back(Vector2Add(GetTailPosition(), direction));
            addingSegment = false;
        }
        else {
            if (IsTimeElapsed(updateTime))
                Move();
        }
        HandleInput();
    }

    void AddSegment() {
        addingSegment = true;
    }

    std::deque<Vector2> GetBody() {
        return body;
    }

    Vector2 GetTailPosition() {
        return body[body.size() - 1];
    }

    Vector2 GetHeadPosition() {
        return body[0];
    }

    void Reset() {
        body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
        direction = { 1, 0 };
    }
    
    void HandleInput() {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) && (prevDirection.y != 1) && (direction.y != 1)) {
            direction = { 0, -1 };
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) && (prevDirection.y != -1) && (direction.y != -1)) {
            direction = { 0, 1 };
        }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) && (prevDirection.x != -1) && (direction.x != -1)) {
            direction = { 1, 0 };
        }
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) && (prevDirection.x != 1) && (direction.x != 1)) {
            direction = { -1, 0 };
        }
    }

    void Move() {
        body.pop_back();
        body.push_front(Vector2Add(body[0], direction));
        prevDirection = direction;
    }

    void Draw() {
        for (unsigned int i = 0; i < body.size(); i++) {
            Rectangle snakeSegment = Rectangle{
                offset + body[i].x * cellSize,
                offset + body[i].y * cellSize,
                cellSize,
                cellSize
            };
            DrawRectangleRounded(snakeSegment, 0.5, 6, LIGHT_COLOR);
        }
    }
};

class Game {
private:
    Snake snake = Snake();
    Food food = Food();

    Sound eatSound;
    Sound wallSound;

    unsigned int score = 0;

    bool isRunning = true;
public:
    Game() {
        food.SetPosition(food.GenerateRandomPosition(snake.GetBody()));
        InitAudioDevice();
        eatSound = LoadSound("audio/eat.mp3");
        wallSound = LoadSound("audio/wall.mp3");
    }

    ~Game() {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    unsigned int GetScore() {
        return score;
    }

    void Update() {
        if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT) ||
            IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_A) ||
            IsKeyPressed(KEY_SPACE)) && (!isRunning)) {
            snake.Reset();
            food.SetPosition(food.GenerateRandomPosition(snake.GetBody()));
            score = 0;
            isRunning = true;
        }
        if (isRunning) {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithBody();
        }
    }

    void CheckCollisionWithFood() {
        if (Vector2Equals(snake.GetHeadPosition(), food.GetPosition())) {
            snake.AddSegment();
            food.SetPosition(food.GenerateRandomPosition(snake.GetBody()));
            score++;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionWithEdges() {
        if (snake.GetHeadPosition().x == cellsWidth || snake.GetHeadPosition().x == -1) {
            GameOver();
        }
        if (snake.GetHeadPosition().y == cellsHeight || snake.GetHeadPosition().y == -1) {
            GameOver();
        }
    }

    void CheckCollisionWithBody() {
        std::deque<Vector2> headlessBody = snake.GetBody();
        headlessBody.pop_front();

        if (ElementInDeque(headlessBody, snake.GetHeadPosition())) {
            GameOver();
        }
    }

    void GameOver() {
        isRunning = false;
        PlaySound(wallSound);
    }

    void Draw() {
        if (isRunning) {
            food.Draw();
            snake.Draw();
        }
        else {
            DrawText("Game Over", screenWidth / 2 - 255, screenHeight / 2 - 80, 100, LIGHT_COLOR);
        }
    }
};

int main() {
    InitWindow(screenWidth, screenHeight, "RaySnake");
    SetTargetFPS(165);
    
    Game game = Game();

    while (!WindowShouldClose()) {
        game.Update();

        BeginDrawing();
        ClearBackground(DARK_COLOR);

        DrawRectangleLinesEx(Rectangle{offset - 5, offset - 5, cellSize * cellsWidth + 10, cellSize * cellsHeight + 10}, 5, LIGHT_COLOR);
        DrawText("RaySnake", offset - 5, 20, 40, LIGHT_COLOR);
        DrawText(TextFormat("%i", game.GetScore()), offset - 5, offset + cellSize * cellsHeight + 10, 40, LIGHT_COLOR);
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}