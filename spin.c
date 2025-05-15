#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define WINDOW_SIZE 800
#define START 0
#define END 330
#define SPEED_LIMIT 100.0f

const Vector2 center = { WINDOW_SIZE / 2.f, WINDOW_SIZE / 2.f };
const Vector2 gravity = { 0.0f, 100.0f };

typedef struct {
    float start;
    float end;
    float inner_r;
    float outer_r;
    Vector2 center;
    float vel;
} Ring;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float radius;
    float mass;
    Color color;
} Ball;

Vector2 vector2mul(Vector2 vec1, Vector2 vec2) {
    return (Vector2){ vec1.x * vec2.x, vec1.y * vec2.y };
}

Vector2 vector2scale(Vector2 vec, float value) {
    return (Vector2){ vec.x * value, vec.y * value };
}

Vector2 vector2add(Vector2 vec1, Vector2 vec2) {
    return (Vector2){ vec1.x + vec2.x, vec1.y + vec2.y };
}

float vector2dot(Vector2 vec1, Vector2 vec2) {
    return vec1.x * vec2.x + vec1.y * vec2.y; 
}

float vector2length(Vector2 vec) {
    return sqrtf(vec.x * vec.x + vec.y * vec.y);
}

Vector2 vector2normalize(Vector2 vec) {
    float len = vector2length(vec);
    if (len == 0) return (Vector2){ 0.0f, 0.0f };
    return (Vector2){ vec.x / len, vec.y / len };
}

Vector2 vector2reflect(Vector2 vec, Vector2 normal) {
    Vector2 n = vector2normalize(normal);
    float dot = vector2dot(vec, n);
    return (Vector2) {
        vec.x - 2.0f  * dot * n.x,
        vec.y - 2.0f * dot * n.y
    };
}

Vector2 vector2subtract(Vector2 vec1, Vector2 vec2) {
    return (Vector2){ vec1.x - vec2.x, vec1.y - vec2.y };
}

float distance(Vector2 vec1, Vector2 vec2) {
    float first = (vec1.x - vec2.x) * (vec1.x - vec2.x);
    float second = (vec1.y - vec2.y) * (vec1.y - vec2.y);
    return sqrtf(first + second);
}

void draw_ring(Ring ring) {
    DrawRing(ring.center, ring.inner_r, ring.outer_r, ring.start, ring.end, 0, BLACK);
}

void draw_ball(Ball ball) {
    DrawCircleV(ball.pos, ball.radius, ball.color);
}

void spin_ring(Ring *ring, float dt) {
    ring->start += ring->vel * dt;
    ring->end += ring->vel * dt;
}

bool check_collision(Ball ball, Ring ring) {
    Vector2 to_ball = vector2subtract(ball.pos, ring.center); 
    float dist = vector2length(to_ball); 
    float ring_width = ring.outer_r - ring.inner_r;
    if (dist < ring.inner_r - ball.radius + ring_width || dist > ring.outer_r + ball.radius - ring_width) 
        return false;
    float angle = atan2f(to_ball.y, to_ball.x) * (180.0 / PI); 
    if (angle < 0) angle += 360.0f;

    float start = fmodf(ring.start, 360.0f); 
    float end = fmodf(ring.end, 360.0f);
    if (start < 0) start += 360.0f;
    if (end < 0) end += 360.0f;

    if (start < end) return angle >= start && angle <= end;
    return angle >= start || angle <= end;
}

Ring *generate_rings(int n) {
    Ring *rings_arr = (Ring *)malloc(n * sizeof(Ring));
    float init_vel = 5;
    float inner_r = 70;
    float outer_r = 65;
    for (int i = 0; i < n; i++) {
        rings_arr[i] = (Ring){ START, END, inner_r, outer_r, center, init_vel * (i + 1) };
        inner_r += 40;
        outer_r += 40;
    }
    return rings_arr;
}

void update_color(Ball *ball) {
    ball->color.r = GetRandomValue(0, 255);
    ball->color.g = GetRandomValue(0, 255);
    ball->color.b = GetRandomValue(0, 255);
}

void update_ball(Ball *ball, Ring *rings, int n, float dt) {
    Vector2 next_pos = vector2add(ball->pos, vector2scale(ball->vel, dt));
    bool bounced = false;

    for (int i = 0; i < n; i++) {
        Ball test_ball = *ball;
        test_ball.pos = next_pos;
        if (check_collision(test_ball, rings[i])) {
            Vector2 to_ball = vector2subtract(ball->pos, rings[i].center);
            Vector2 normal = vector2normalize(to_ball);
            ball->vel = vector2reflect(ball->vel, normal);
            update_color(ball);
            bounced = true;
            break;
        }
    }

    if (!bounced) {
        ball->vel = vector2add(ball->vel, vector2scale(gravity, dt));
        ball->pos = next_pos;
    }
}

int main(int argc, char **argv) {
    InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Centrifuuuga");
    int rings_n = 9;
    Color ball_color = { 0, 10, 20, 255 };
    Ring *rings_arr = generate_rings(rings_n);
    Ball ball = { center, (Vector2){ 15.0f, 15.0f }, 8, 5, ball_color };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        ClearBackground(RAYWHITE);
        BeginDrawing();
        bool bounced = false;
        draw_ball(ball);

        Ball check_ball = ball;
        check_ball.pos  = vector2add(check_ball.pos, vector2scale(ball.vel, dt));

        for (int i = 0; i < rings_n; i++) {
            if (distance(ball.pos, center) > rings_arr[i].inner_r) {
                for (int j = i; j < rings_n - 1; j++) rings_arr[j] = rings_arr[j + 1];
                rings_n--;
                rings_arr = realloc(rings_arr, rings_n * sizeof(Ring));
                ball.vel = vector2scale(ball.vel, 1.2);
                ball.radius *= 1.2;
            }
        }

        for (int i = 0; i < rings_n; i++) {
            draw_ring(rings_arr[i]);
            spin_ring(&rings_arr[i], dt);
        }
        
        update_ball(&ball, rings_arr, rings_n, dt);

        EndDrawing();
    }
    free(rings_arr);
    CloseWindow();
    return 0;
}
