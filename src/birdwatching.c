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

#define MAX_BOIDS 600
#define MAX_NEIGHBOURS 30

typedef struct {
    Vector3 position;
    Vector3 velocity;
    int neighbourBoidIndexes[MAX_NEIGHBOURS];
} Boid;

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
Camera camera = { 0 };
Vector3 cubePosition = { 0 };
int numBoids = MAX_BOIDS;
int maxNeighbours = MAX_NEIGHBOURS;
Boid boids[MAX_BOIDS] = { 0 };

// Shader
float timeCounter = 0.0f;
float grainIntensity = 0.2f;
RenderTexture2D target;
Shader grainShader;

Vector3 worldBounds = {
    .x = 50.0f,
    .y = 10.0f,
    .z = 10.0f
};


//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);          // Update and draw one frame
static void InitBoids(void);
static void UpdateBoidNeighbours(void);
static void KeepWithinBounds(void);
static void SteerSeparation(void);
static void SteerAlignment(void);
static void SteerCohesion(void);
static void ConstrainSpeed(void);
static void UpdateBoidPosition(void);

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "raylib - birdwatching");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    grainShader = LoadShader(0, "film_grain.fs");
    target = LoadRenderTexture(screenWidth, screenHeight);

    float grainIntensity = 0.1f;
    float timeCounter = 0.0f;

    InitBoids();

    camera.position = (Vector3){ 0.0f, -20.0f, 50.0f };
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
    UnloadShader(grainShader);
    UnloadRenderTexture(target);
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

static void InitBoids(void) {
    for (int i = 0; i < numBoids; i++) {
        boids[i].position.x = GetRandomValue(- worldBounds.x,  worldBounds.x);
        boids[i].position.y = GetRandomValue(- worldBounds.y,  worldBounds.y);
        boids[i].position.z = GetRandomValue(-worldBounds.z, worldBounds.z);
        boids[i].velocity.x = GetRandomValue(-1.0, 1.0);
        boids[i].velocity.y = GetRandomValue(-1.0, 1.0);
        boids[i].velocity.z = GetRandomValue(-1.0, 1.0);

        // Reset neighbours
        for (int y = 0; y < maxNeighbours; y++) {
            boids[i].neighbourBoidIndexes[y] = -1;
        }
    }
}

static void UpdateBoidNeighbours(void) {
    for (int i = 0; i < numBoids; i++) {
        // Reset neighbours
        for (int y = 0; y < maxNeighbours; y++) {
            boids[i].neighbourBoidIndexes[y] = -1;
        }

        int neighbourIndex = 0;
		for (int y = 0; y < numBoids; y++) {
            if (i == y) {
                continue;
            }

            float distance = Vector3Distance(boids[i].position, boids[y].position);
            if (distance < 5.0f) {
                // Check alignment using dot product
                if (Vector3DotProduct(boids[i].velocity, boids[y].velocity) > 0) {
					boids[i].neighbourBoidIndexes[neighbourIndex] = y;
					neighbourIndex++;
                }

                if (neighbourIndex == 10) {
                    break;
                }
            }
		}
    }
}

static void KeepWithinBounds(void) {
    float turnFactor = 0.1f; // Smaller value for smoother turning
    for (int i = 0; i < numBoids; i++) {
        Vector3 steering = Vector3Zero();

        if (boids[i].position.x > worldBounds.x) {
            steering.x = -1.0f; // Steer left
        }
        else if (boids[i].position.x < -worldBounds.x) {
            steering.x = 1.0f; // Steer right
        }

        if (boids[i].position.y > worldBounds.y) {
            steering.y = -1.0f; // Steer down
        }
        else if (boids[i].position.y < -worldBounds.y) {
            steering.y = 1.0f; // Steer up
        }

        if (boids[i].position.z > worldBounds.z) {
            steering.z = -1.0f; // Steer back
        }
        else if (boids[i].position.z < -worldBounds.z) {
            steering.z = 1.0f; // Steer forward
        }

        // Normalize the steering vector and scale it
        if (Vector3Length(steering) > 0) {
            steering = Vector3Normalize(steering);
            steering = Vector3Scale(steering, turnFactor);
            boids[i].velocity = Vector3Add(boids[i].velocity, steering);
        }
    }
}

static void SteerSeparation(void) {
    float avoidFactor = 0.02;
    for (int i = 0; i < numBoids; i++) {
        Vector3 direction = Vector3Zero();
		int neighbourCount = 0;
		for (int y = 0; y < maxNeighbours; y++) {
            if (boids[i].neighbourBoidIndexes[y] > -1) {
                int neighborIndex = boids[i].neighbourBoidIndexes[y];
                Vector3 neighborPosition = boids[neighborIndex].position;

                direction.x += boids[i].position.x - boids[neighborIndex].position.x;
                direction.y += boids[i].position.y - boids[neighborIndex].position.y;
                direction.z += boids[i].position.z - boids[neighborIndex].position.z;
                neighbourCount++;
            }
		}

        if (neighbourCount > 0) {
            direction = Vector3Scale(direction, 1.0f / neighbourCount);  // Normalize
            direction = Vector3Normalize(direction);  // Ensure unit length
            boids[i].velocity = Vector3Add(boids[i].velocity, Vector3Scale(direction, avoidFactor));
        }
    }
}

static void SteerAlignment(void) {
    float matchingFactor = 0.05;
    for (int i = 0; i < numBoids; i++) {
        Vector3 velocityAvg = Vector3Zero();
		int neighbourCount = 0;
        for (int y = 0; y < maxNeighbours; y++) {
            if (boids[i].neighbourBoidIndexes[y] > -1) {
                int neighbourIndex = boids[i].neighbourBoidIndexes[y];
                
                velocityAvg.x += boids[neighbourIndex].velocity.x;
                velocityAvg.y += boids[neighbourIndex].velocity.y;
                velocityAvg.z += boids[neighbourIndex].velocity.z;
                neighbourCount++;
            }
        }

        if (neighbourCount > 0) {
            velocityAvg.x = velocityAvg.x / neighbourCount;
            velocityAvg.y = velocityAvg.y / neighbourCount;
            velocityAvg.z = velocityAvg.z / neighbourCount;
        }

        boids[i].velocity.x += (velocityAvg.x - boids[i].velocity.x) * matchingFactor;
        boids[i].velocity.y += (velocityAvg.y - boids[i].velocity.y) * matchingFactor;
        boids[i].velocity.z += (velocityAvg.z - boids[i].velocity.z) * matchingFactor;
    }
}

static void SteerCohesion(void) {
    float centeringFactor = 0.004;
    for (int i = 0; i < numBoids; i++) {
        Vector3 positionAvg = Vector3Zero();
        int neighbourCount = 0;
        for (int y = 0; y < maxNeighbours; y++) {
            if (boids[i].neighbourBoidIndexes[y] > -1) {
                int neighbourIndex = boids[i].neighbourBoidIndexes[y];
                positionAvg = Vector3Add(positionAvg, boids[neighbourIndex].position);
                neighbourCount++;
            }
        }

        if (neighbourCount > 0) {
            positionAvg.x /= neighbourCount;
            positionAvg.y /= neighbourCount;
            positionAvg.z /= neighbourCount;
        }

        boids[i].velocity.x += (positionAvg.x - boids[i].position.x) * centeringFactor;
        boids[i].velocity.y += (positionAvg.y - boids[i].position.y) * centeringFactor;
        boids[i].velocity.z += (positionAvg.z - boids[i].position.z) * centeringFactor;
    }
}

static void ConstrainSpeed(void) {
    float maxSpeed = 3.0;
    float minSpeed = 2.0;
    for (int i = 0; i < numBoids; i++) {
        float speed = sqrt(
            boids[i].velocity.x * boids[i].velocity.x + 
            boids[i].velocity.y * boids[i].velocity.y + 
            boids[i].velocity.z * boids[i].velocity.z);

        if (speed > maxSpeed) {
            boids[i].velocity = Vector3Scale(Vector3Normalize(boids[i].velocity), maxSpeed);
        }
        else if (speed < minSpeed) {
            boids[i].velocity = Vector3Scale(Vector3Normalize(boids[i].velocity), minSpeed);
        }
    }
}

static void UpdateBoidPosition(void) {
    for (int i = 0; i < numBoids; i++) {
        Vector3 displacement = Vector3Scale(boids[i].velocity, 3 * GetFrameTime());
        boids[i].position = Vector3Add(boids[i].position, displacement);
    }
}

// Update and draw game frame
static void UpdateDrawFrame(void)
{
    timeCounter += GetFrameTime();

    // Update
    //----------------------------------------------------------------------------------
    UpdateCamera(&camera, CAMERA_FREE);
    UpdateBoidNeighbours();
    SteerSeparation();
    SteerAlignment();
    SteerCohesion();
    KeepWithinBounds();
    ConstrainSpeed();
    UpdateBoidPosition();
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginTextureMode(target);

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

			for (int i = 0; i < numBoids; i++) {
                DrawSphere(boids[i].position, 0.08, DARKGRAY);
               // DrawLine3D(boids[i].position, Vector3Add(boids[i].velocity, boids[i].position), RED);

                /*
                for (int y = 0; y < 10; y++) {
                    if (boids[i].neighbourBoidIndexes[y] > -1) {
                        DrawLine3D(boids[i].position, boids[boids[i].neighbourBoidIndexes[y]].position, LIGHTGRAY);
                    }
                }
                */
            }
            DrawPlane((Vector3){0.0f, -20.0f, 0.0f}, (Vector2) { 300.0f, 100.0f }, RED);

        EndMode3D();

        DrawFPS(10, 10);

    EndTextureMode();

    BeginDrawing();
		ClearBackground(BLACK);

        BeginShaderMode(grainShader);
            SetShaderValue(grainShader, GetShaderLocation(grainShader, "time"), &timeCounter, SHADER_UNIFORM_FLOAT);
            SetShaderValue(grainShader, GetShaderLocation(grainShader, "grainIntensity"), &grainIntensity, SHADER_UNIFORM_FLOAT);
            DrawTextureRec(target.texture, (Rectangle) { 0, 0, GetScreenWidth(), -GetScreenHeight() }, (Vector2) { 0, 0 }, WHITE);
        EndShaderMode();
    EndDrawing();
    //----------------------------------------------------------------------------------
}
