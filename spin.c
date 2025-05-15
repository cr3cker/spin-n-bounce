#include <math.h>
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define START 0
#define END 330
#define SPEED_LIMIT 100.0f
#define SIZE 800
#define SUBTICK 5

const Vector2 gravity = { 0.0f, 150.0f };
const Vector2 center = { SIZE / 2.0f, SIZE / 2.0f };

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
    bool bounced;
    float spawn_cooldown;
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
    DrawRing(ring.center, ring.inner_r, ring.outer_r, ring.start, ring.end, 0, GRAY);
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
    float init_vel = 10;
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

void update_ball(Ball *ball, Ring *rings, int rings_n, float dt) {
    ball->bounced = false;
    ball->vel = vector2add(ball->vel, vector2scale(gravity, dt));
    Vector2 next_pos = vector2add(ball->pos, vector2scale(ball->vel, dt));

    for (int i = 0; i < rings_n; i++) {
        Ball test_ball = *ball;
        test_ball.pos = next_pos;
        if (check_collision(test_ball, rings[i])) {
            Vector2 to_ball = vector2subtract(ball->pos, rings[i].center);
            Vector2 normal = vector2normalize(to_ball);
            ball->vel = vector2reflect(ball->vel, normal);
            update_color(ball);
            ball->bounced = true;
            return;
        }
    }

    ball->pos = next_pos;
}

Ring *remove_ring(Ring *rings_arr, int *rings_n) {
    for (int i = 0; i < *rings_n - 1; i++) rings_arr[i] = rings_arr[i + 1];
    (*rings_n)--;
    rings_arr = realloc(rings_arr, (*rings_n) * sizeof(Ring));
    return rings_arr;
}

// this crazy stuff is taken from stackoverflow, after reading a proof, I cried and left the article in agony
void elastic_collision(Ball *b1, Ball *b2) {
    Vector2 x1 = b1->pos;
    Vector2 x2 = b2->pos;
    Vector2 v1 = b1->vel;
    Vector2 v2 = b2->vel;
    float m1 = b1->mass;
    float m2 = b2->mass;

    Vector2 delta_pos = vector2subtract(x1, x2);
    Vector2 delta_vel = vector2subtract(v1, v2);
    float dist_squared = vector2dot(delta_pos, delta_pos);

    float dot_product = vector2dot(delta_vel, delta_pos);

    float factor1 = (2.0f * m2 / (m1 + m2)) * (dot_product / dist_squared);
    Vector2 correction1 = vector2scale(delta_pos, factor1);
    b1->vel = vector2subtract(v1, correction1);

    delta_pos = vector2subtract(x2, x1);
    delta_vel = vector2subtract(v2, v1);
    dot_product = vector2dot(delta_vel, delta_pos);
    float factor2 = (2.0f * m1 / (m1 + m2)) * (dot_product / dist_squared);
    Vector2 correction2 = vector2scale(delta_pos, factor2);
    b2->vel = vector2subtract(v2, correction2);
}

void decrease_cooldown(Ball *balls_arr, int balls_n, float dt) {
    for (int i = 0; i < balls_n; i++) {
        if (balls_arr[i].spawn_cooldown > 0) {
            balls_arr[i].spawn_cooldown -= dt;
        }
    }
}

void add_ball(int *balls_n, int *rings_n, Ball **balls_arr, Ring **rings_arr) {
    for (int j = 0; j < *balls_n; j++) {
        int i = 0;
        while (i < *rings_n) {
            if (distance((*balls_arr)[j].pos, center) > (*rings_arr)[i].inner_r - (*balls_arr)[j].radius) {
                (*balls_n)++;
                *balls_arr = realloc(*balls_arr, (*balls_n) * sizeof(Ball));

                Ball new_ball = (*balls_arr)[j];
                new_ball.spawn_cooldown = 1.0f;
                new_ball.pos.x += GetRandomValue(-10, 10);
                new_ball.pos.y += GetRandomValue(-10, 10);

                (*balls_arr)[*balls_n - 1] = new_ball;
                *rings_arr = remove_ring(*rings_arr, rings_n);
                (*balls_arr)[j].radius *= 1.2f;
                (*balls_arr)[j].vel = vector2scale((*balls_arr)[j].vel, 1.1f);
                break;
            } else {
                i++;
            }
        }
    }
}

void handle_ball_collisions(Ball *balls, int balls_n) {
    for (int i = 0; i < balls_n; i++) {
        for (int j = i + 1; j < balls_n; j++) {
            if (balls[i].spawn_cooldown <= 0 && balls[j].spawn_cooldown <= 0 &&
                CheckCollisionCircles(balls[i].pos, balls[i].radius, balls[j].pos, balls[j].radius)) {
                elastic_collision(&balls[i], &balls[j]);
            }
        }
    }
}

int main() {
    InitWindow(800, 800, "Centrifuuuga");
    Vector2 center = { SIZE / 2.f, SIZE / 2.f };
    int rings_n = 9;
    int balls_n = 1;
    Color ball_color = { 0, 10, 20, 255 };
    Ring *rings_arr = generate_rings(rings_n);
    Ball *balls_arr = (Ball *)malloc(balls_n * sizeof(Ball));
    Ball ball = { center, (Vector2){ 10.0f, 10.0f }, 8, 10, ball_color, false, 1.0f };
    balls_arr[0] = ball;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        ClearBackground(BLACK);
        BeginDrawing();

        decrease_cooldown(balls_arr, balls_n, dt);

        handle_ball_collisions(balls_arr, balls_n);
        add_ball(&balls_n, &rings_n, &balls_arr, &rings_arr);

        for (int i = 0; i < balls_n; i++) {
            draw_ball(balls_arr[i]);
            update_ball(&balls_arr[i], rings_arr, rings_n, dt);
        }

        for (int i = 0; i < rings_n; i++) {
            draw_ring(rings_arr[i]);
            spin_ring(&rings_arr[i], dt);
        }
        
        EndDrawing();
    }
    free(rings_arr);
    CloseWindow();
    return 0;
}
