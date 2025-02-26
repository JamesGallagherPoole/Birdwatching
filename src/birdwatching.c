/*******************************************************************************************
*
*   project_name
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

typedef struct {
    Vector3 position;
    Vector3 velocity;
    int neighbourBoidIndexes[10];
} Boid;

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
Camera camera = { 0 };
Vector3 cubePosition = { 0 };
int numBoids = 200;
Boid boids[200] = { 0 };

Vector3 worldBounds = {
    .x = 10.0f,
    .y = 10.0f,
    .z = 10.0f
};


//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);          // Update and draw one frame
static void InitBoids(void);
static void UpdateBoidMovementVector(void);
static void UpdateBoidPosition(void);

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib - project_name");
    InitBoids();

    camera.position = (Vector3){ 3.0f, 3.0f, 2.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

static void InitBoids(void) {
    for (int i = 0; i <= numBoids; i++) {
        boids[i].position.x = GetRandomValue(- worldBounds.x,  worldBounds.x);
        boids[i].position.y = GetRandomValue(- worldBounds.y,  worldBounds.y);
        boids[i].position.z = GetRandomValue(-worldBounds.z, worldBounds.z);
        boids[i].velocity.x = 1.0;
    }
}

static void UpdateBoidMovementVector(void) {
    for (int i = 0; i <= numBoids; i++) {
        // Reset neighbours
        for (int y = 0; y <= 10; y++) {
            boids[i].neighbourBoidIndexes[y] = 0;
        }

        int neighbourIndex = 0;
		for (int y = 0; y <= numBoids; y++) {
            if (i == y) {
                continue;
            }

            float distance = Vector3Distance(boids[i].position, boids[y].position);
            if (distance < 1.0f) {
                boids[i].neighbourBoidIndexes[neighbourIndex] = y;
                neighbourIndex++;
            }
		}
    }
}

static void UpdateBoidPosition(void) {
    for (int i = 0; i <= numBoids; i++) {
        Vector3 displacement = Vector3Scale(boids[i].velocity, GetFrameTime());
        boids[i].position = Vector3Add(boids[i].position, displacement);
    }
}

// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdateCamera(&camera, CAMERA_FREE);
    UpdateBoidMovementVector();
    UpdateBoidPosition();
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

			for (int i = 0; i < numBoids; i++) {
                DrawSphere(boids[i].position, 0.1, DARKGRAY);
            }
            DrawGrid(10, 1.0f);

        EndMode3D();

        DrawText("Welcome to raylib basic sample", 10, 40, 20, DARKGRAY);

        DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
