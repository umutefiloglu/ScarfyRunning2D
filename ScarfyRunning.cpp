#include "raylib.h"
#include <cmath>

// Animation-related variables.
struct AnimData
{
    Rectangle rect;
    Vector2 pos;
    int frame;
    float updateTime;
    float runningTime;
};

AnimData UpdateAnimData(AnimData data, float deltaTime, int maxFrame)
{
    data.runningTime += deltaTime;
    if (data.runningTime >= data.updateTime)
    {
        data.runningTime = 0;
        data.rect.x = data.frame * data.rect.width;
        data.frame++;
        if (data.frame > maxFrame)
        {
            data.frame = 0;
        }
    }
    return data;
}

// Check if player is on ground
bool IsOnGround(AnimData data, int winHeight)
{
    return data.pos.y >= winHeight - data.rect.height;
}

int main()
{
    // App Setting
    const int target_fps = 60;
    SetTargetFPS(target_fps); // But does NOT guarantee this value

    // Window
    // ind0 is width, ind1 is height
    int windowDimensions[2];
    windowDimensions[0] = 800;
    windowDimensions[1] = 600;
    const char *window_name{"Scarfy Running"};
    // Initialize window
    InitWindow(windowDimensions[0], windowDimensions[1], window_name);

    Texture2D backgroundText = LoadTexture("textures/far-buildings.png");
    Texture2D midgroundText = LoadTexture("textures/back-buildings.png");
    Texture2D foregroundText = LoadTexture("textures/foreground.png");
    float bgX{0}; // Background X
    float mgX{0}; // Middleground X
    float fgX{0}; // Foreground X

    // Player Variables
    // Player velocity
    int player_velocity{10};
    // Player jump force pixels/s
    int player_jump_force{-600};
    // Player Constraints
    bool is_player_in_Air{false};
    // Player Texture
    Texture2D playerText = LoadTexture("textures/scarfy.png");

    AnimData playerData;
    playerData.rect.width = playerText.width / 6; // Because there are 6 sprites in a sheet;
    playerData.rect.height = playerText.height;
    playerData.rect.x = 0;
    playerData.rect.y = 0;
    playerData.pos.x = windowDimensions[0] / 2 - playerData.rect.width / 2;
    playerData.pos.y = windowDimensions[1] - playerData.rect.height;
    playerData.frame = 0; // Current anim. frame
    playerData.updateTime = 1.0 / 12.0;
    playerData.runningTime = 0.0;

    // Enemy Variables
    // Enemy Texture
    Texture2D enemyText = LoadTexture("textures/12_nebula_spritesheet.png");

    const int sizeOfEnemies = 3;
    AnimData enemiesData[sizeOfEnemies]{};
    for (int i = 0; i < sizeOfEnemies; i++)
    {
        enemiesData[i].rect.x = 0.0;
        enemiesData[i].rect.y = 0.0;
        enemiesData[i].rect.width = enemyText.width / 8;
        enemiesData[i].rect.height = enemyText.width / 8;
        enemiesData[i].pos.y = windowDimensions[1] - enemyText.height / 8;
        enemiesData[i].frame = 0;
        enemiesData[i].runningTime = 0;
        enemiesData[i].updateTime = 0.0;
        enemiesData[i].pos.x = windowDimensions[0] + (i * 300);
    }

    // Enemy velocity (pixels/second)
    int enemyVelocity{-200};

    // Finish Line
    float finishLine{enemiesData[sizeOfEnemies - 1].pos.x};

    // World
    // acceleration due to gravity (pixel/s)/s
    const int gravity{1'000}; // ' ignored by compiler.

    /*
        * if window width 5 && texture width 2.5
                then scale is 2 => window width / texture width
                pick lower value
        */
    float bgScale{1.0};
    if (backgroundText.width < backgroundText.height)
    {
        bgScale = (float)windowDimensions[0] / backgroundText.width;
    }
    else
    {
        bgScale = (float)windowDimensions[1] / backgroundText.height;
    }
    float mgScale{1.0};
    if (midgroundText.width < midgroundText.height)
    {
        mgScale = (float)windowDimensions[0] / midgroundText.width;
    }
    else
    {
        mgScale = (float)windowDimensions[1] / midgroundText.height;
    }
    float fgScale{1.0};
    if (foregroundText.width < foregroundText.height)
    {
        fgScale = (float)windowDimensions[0] / foregroundText.width;
    }
    else
    {
        fgScale = (float)windowDimensions[1] / foregroundText.height;
    }

    bool collision{};
    bool crossedFinish{};

    // While loop executes each frame
    while (WindowShouldClose() == false)
    {
        // App Settings
        const float deltaTime{GetFrameTime()}; // Time since last frame

        // Start Drawing
        BeginDrawing();
        ClearBackground(WHITE);

        // Move background texture
        bgX -= 100 * deltaTime;
        if (bgX <= -backgroundText.width * bgScale)
        {
            bgX = 0;
        }
        mgX -= 150 * deltaTime;
        if (mgX <= -midgroundText.width * mgScale)
        {
            mgX = 0;
        }
        fgX -= 200 * deltaTime;
        if (fgX <= -foregroundText.width * fgScale)
        {
            fgX = 0;
        }

        // Draw Background 1
        Vector2 bg1Pos{bgX, 0.0f};
        // Draw Midground 1
        Vector2 mg1Pos{mgX, 0.0f};
        // Draw Foreground 1
        Vector2 fg1Pos{fgX, 0.0f};
        // Draw Background 2
        Vector2 bg2Pos{bgX + backgroundText.width*bgScale, 0.0f};
        // Draw Midground 2
        Vector2 mg2Pos{mgX + midgroundText.width*mgScale, 0.0f};
        // Draw Foreground 2
        Vector2 fg2Pos{fgX + foregroundText.width*fgScale, 0.0f};

        DrawTextureEx(backgroundText, bg1Pos, 0.0, bgScale, WHITE);
        DrawTextureEx(backgroundText, bg2Pos, 0.0, bgScale, WHITE);

        DrawTextureEx(midgroundText, mg1Pos, 0.0, mgScale, WHITE);
        DrawTextureEx(midgroundText, mg2Pos, 0.0, mgScale, WHITE);

        DrawTextureEx(foregroundText, fg1Pos, 0.0, fgScale, WHITE);
        DrawTextureEx(foregroundText, fg2Pos, 0.0, fgScale, WHITE);

        // Apply gravity
        // Player on ground
        if (IsOnGround(playerData, windowDimensions[1]))
        {
            player_velocity = 0;
            is_player_in_Air = false;
        }
        // Player on air
        else
        {
            is_player_in_Air = true;
            player_velocity += gravity * deltaTime;
        }

        // Player Jump
        if (IsKeyPressed(KEY_SPACE) && !is_player_in_Air)
        {
            player_velocity += player_jump_force;
        }

        // Update player position
        playerData.pos.y += player_velocity * deltaTime;

        for (int i = 0; i < sizeOfEnemies; i++)
        {
            // Update Enemy Position
            enemiesData[i].pos.x += enemyVelocity * deltaTime;
        }

        // Update finish line
        finishLine += enemyVelocity * deltaTime;

        if (!is_player_in_Air)
        {
            // Update animation frame
            playerData = UpdateAnimData(playerData, deltaTime, 5);
        }

        for (int i = 0; i < sizeOfEnemies; i++)
        {
            enemiesData[i] = UpdateAnimData(enemiesData[i], deltaTime, 7);
        }

        // Detect collision
        for (AnimData enemy: enemiesData)
        {
            float pad{40};
            Rectangle enemyRect{
                enemy.pos.x + pad,
                enemy.pos.y + pad,
                enemy.rect.width - 2*pad,
                enemy.rect.height - 2*pad
            };
            Rectangle playerRect{
                playerData.pos.x,
                playerData.pos.y,
                playerData.rect.width,
                playerData.rect.height
            };
            if (CheckCollisionRecs(enemyRect, playerRect))
            {
                collision = true;
            }
        }
        
        if (playerData.pos.x >= finishLine)
        {
            crossedFinish = true;
        }
        

        if (collision)
        {
            // Game Lost
            DrawText("Game Over!", windowDimensions[0]/4, windowDimensions[1]/2, 50, RED);
        }
        else if (crossedFinish)
        {
            // Game Won
            DrawText("You Win!", windowDimensions[0]/4, windowDimensions[1]/2, 50, GREEN);
        }
        // Game runs
        else
        {
            // Draw Player
            DrawTextureRec(playerText, playerData.rect, playerData.pos, WHITE);

            for (int i = 0; i < sizeOfEnemies; i++)
            {
                // Draw Enemy
                DrawTextureRec(enemyText, enemiesData[i].rect, enemiesData[i].pos, WHITE);
            }
        }

        // Stop Drawing
        EndDrawing();
    }

    // Memory Management
    // Unload textures
    UnloadTexture(playerText);
    UnloadTexture(enemyText);
    UnloadTexture(backgroundText);
    UnloadTexture(midgroundText);
    UnloadTexture(foregroundText);
    // Shuts the window correctly
    CloseWindow();
}