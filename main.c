#include <gba.h>
#include <math.h>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160
#define MAP_WIDTH 8
#define MAP_HEIGHT 8

int worldMap[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1}
};

volatile u16* fb = (u16*)MODE3_FB;

void drawVLine(int x, int y1, int y2, u16 color) {
    if (x < 0 || x >= SCREEN_WIDTH) return;
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    if (y1 < 0) y1 = 0;
    if (y2 >= SCREEN_HEIGHT) y2 = SCREEN_HEIGHT - 1;
    for (int y = y1; y <= y2; y++) {
        fb[y * SCREEN_WIDTH + x] = color;
    }
}

int main(void) {
    irqInit();
    irqEnable(IRQ_VBLANK);
    REG_DISPCNT = MODE_3 | BG2_ON;

    float posX = 4.5f, posY = 4.5f;
    float dirX = -1.0f, dirY = 0.0f;
    float planeX = 0.0f, planeY = 0.66f;

    while (1) {
        VBlankIntrWait();
        scanKeys();
        u16 keys = keysHeld();

        float moveSpeed = 0.28f;
        float rotSpeed = 0.19f;

        // Movement
        if (keys & KEY_UP) {
            float nextX = posX + dirX * moveSpeed;
            float nextY = posY + dirY * moveSpeed;
            if (worldMap[(int)posY][(int)nextX] == 0) posX = nextX;
            if (worldMap[(int)nextY][(int)posX] == 0) posY = nextY;
        }
        if (keys & KEY_DOWN) {
            float nextX = posX - dirX * moveSpeed;
            float nextY = posY - dirY * moveSpeed;
            if (worldMap[(int)posY][(int)nextX] == 0) posX = nextX;
            if (worldMap[(int)nextY][(int)posX] == 0) posY = nextY;
        }
        if (keys & KEY_LEFT) {
            float oldDirX = dirX;
            dirX = dirX * cosf(rotSpeed) - dirY * sinf(rotSpeed);
            dirY = oldDirX * sinf(rotSpeed) + dirY * cosf(rotSpeed);
            float oldPlaneX = planeX;
            planeX = planeX * cosf(rotSpeed) - planeY * sinf(rotSpeed);
            planeY = oldPlaneX * sinf(rotSpeed) + planeY * cosf(rotSpeed);
        }
        if (keys & KEY_RIGHT) {
            float oldDirX = dirX;
            dirX = dirX * cosf(-rotSpeed) - dirY * sinf(-rotSpeed);
            dirY = oldDirX * sinf(-rotSpeed) + dirY * cosf(-rotSpeed);
            float oldPlaneX = planeX;
            planeX = planeX * cosf(-rotSpeed) - planeY * sinf(-rotSpeed);
            planeY = oldPlaneX * sinf(-rotSpeed) + planeY * cosf(-rotSpeed);
        }

        // Raycasting and drawing
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            float cameraX = 2.0f * x / SCREEN_WIDTH - 1.0f;
            float rayDirX = dirX + planeX * cameraX;
            float rayDirY = dirY + planeY * cameraX;

            int mapX = (int)posX;
            int mapY = (int)posY;

            float sideDistX, sideDistY;
            float deltaDistX = (rayDirX == 0) ? 1e30 : fabsf(1.0f / rayDirX);
            float deltaDistY = (rayDirY == 0) ? 1e30 : fabsf(1.0f / rayDirY);
            float perpWallDist;

            int stepX, stepY;
            int hit = 0, side = 0;

            if (rayDirX < 0) {
                stepX = -1;
                sideDistX = (posX - mapX) * deltaDistX;
            } else {
                stepX = 1;
                sideDistX = (mapX + 1.0f - posX) * deltaDistX;
            }

            if (rayDirY < 0) {
                stepY = -1;
                sideDistY = (posY - mapY) * deltaDistY;
            } else {
                stepY = 1;
                sideDistY = (mapY + 1.0f - posY) * deltaDistY;
            }

            while (!hit) {
                if (sideDistX < sideDistY) {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                } else {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }
                if (worldMap[mapY][mapX] > 0) hit = 1;
            }

            perpWallDist = (side == 0)
                ? (mapX - posX + (1 - stepX) * 0.5f) / rayDirX
                : (mapY - posY + (1 - stepY) * 0.5f) / rayDirY;

            int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);
            int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
            int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;

            if (drawStart < 0) drawStart = 0;
            if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

            u16 ceilingColor = RGB5(4, 4, 6);
            u16 floorColor   = RGB5(2, 2, 2);
            u16 wallColor    = (side == 1) ? RGB5(15, 15, 15) : RGB5(31, 31, 31);

            drawVLine(x, 0, drawStart - 1, ceilingColor);
            drawVLine(x, drawStart, drawEnd, wallColor);
            drawVLine(x, drawEnd + 1, SCREEN_HEIGHT - 1, floorColor);
        }
    }

    return 0;
}
