#include <windows.h>
#include <GL/glut.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "tutorial4.h" // 假設這些標頭檔是正確的
#include "texture.h"
#include "3dsloader.h"
#include <mmsystem.h>  // 用於播放音效
#pragma comment(lib, "winmm.lib")  // 連結音效庫

#define MAX_VERTICES 8000
#define MAX_POLYGONS 8000
#define M_PI 3.14159

// --- 全局變數定義 (保持不變) ---
GLuint skyboxTex[6];
static char lastKeyStr[32] = "None";
char* faces[6] = {
    "px.bmp", "nx.bmp", "py.bmp", "ny.bmp", "pz.bmp", "nz.bmp"
};
bool musicEnabled = true;  // 控制音樂開關的變數
int musicVolume = 300;  // 音量範圍 0-1000，300表示30%音量
// 攝影機參數
double camAngle = 0.0;
double angle_s = 0.0;
double angle_sx = 0;
float baseRadius;
float shipRadius;

// 遊戲狀態
enum GameState { STATE_START, STATE_PLAYING, STATE_END };
GameState gameState = STATE_START;

// 遊戲參數
int health = 3;
int timeLeft = 25;  // 改為20秒
float score = 0;
bool isFlickering = false;
int flickerTimer = 0;
int gameTime = 0;    // 遊玩時間計數

// 太空船參數
double posX = 0, posY = 0, posZ = 0;
float damping = 0.0f;
float normalDamping = 0.3f;
float collisionDamping = 2.0f;

// 畫面參數
int screen_width = 800; // 調整預設視窗大小，讓畫面更寬敞
int screen_height = 600;

// 3D物件
obj_type2 object;    // 太空船
obj_type2 object1;  // 障礙物
obj_type2 healthObject;  // 血量道具物件

// 障礙物結構
struct Obstacle {
    float x, y, z;
    float scale;
    bool alive;
};
std::vector<Obstacle> obstacles;
const int NUM_OBS = 55;
const float OB_SPEED = 2.5f;
const float SPAWN_DISTANCE = 400.0f;
const float RESET_DISTANCE = 20.0f;

// 血量道具結構
struct HealthPowerup {
    float x, y, z;
    float scale;
    bool alive;
    float rotationY; // 用於旋轉動畫
};
std::vector<HealthPowerup> healthPowerups;
const int NUM_HEALTH_POWERUPS = 15; // 比障礙物少很多
const float HEALTH_SPAWN_DISTANCE = 500.0f;
const float HEALTH_RESET_DISTANCE = 20.0f;

// 立方體定義（保留原有定義）
typedef struct {
    float x, y, z;
}vertex_type;

typedef struct {
    int a, b, c;
}polygon_type;

typedef struct {
    float u, v;
}mapcoord_type;

typedef struct {
    vertex_type vertex[MAX_VERTICES];
    polygon_type polygon[MAX_POLYGONS];
    mapcoord_type mapcoord[MAX_VERTICES];
} obj_type, * obj_type_ptr;

obj_type cube = {
    {
        {-10, -10, 10}, {10, -10, 10}, {10, 10, 10}, {-10, 10, 10},
        {10, -10, -10}, {-10, -10, -10}, {-10, 10, -10}, {10, 10, -10},
        {-10, -10, -10}, {-10, -10, 10}, {-10, 10, 10}, {-10, 10, -10},
        {10, -10, 10}, {10, -10, -10}, {10, 10, -10}, {10, 10, 10},
        {-10, 10, 10}, {10, 10, 10}, {10, 10, -10}, {-10, 10, -10},
        {-10, -10, -10}, {10, -10, -10}, {10, -10, 10}, {-10, -10, 10}
    },
    {
        {0, 1, 2}, {0, 2, 3}, {4, 5, 6}, {4, 6, 7},
        {8, 9,10}, {8,10,11}, {12,13,14}, {12,14,15},
        {16,17,18}, {16,18,19}, {20,21,22}, {20,22,23}
    },
    {
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0},
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0},
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0},
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0},
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0},
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}
    }
};
// 添加播放背景音樂的函式：
void playBackgroundMusic() {
    if (musicEnabled) {
        // 開啟音樂檔案
        mciSendString(TEXT("open \"background.mp3\" type mpegvideo alias bgm"), NULL, 0, NULL);

        // 設定音量 (0-1000)
        char volumeCmd[50];
        sprintf(volumeCmd, "setaudio bgm volume to %d", musicVolume);
        mciSendString(TEXT(volumeCmd), NULL, 0, NULL);

        // 循環播放
        mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);
    }
}

// 停止背景音樂的函式：
void stopBackgroundMusic() {
    mciSendString(TEXT("stop bgm"), NULL, 0, NULL);
    mciSendString(TEXT("close bgm"), NULL, 0, NULL);
}
// --- 初始化 Skybox ---
void initSkybox() {
    glEnable(GL_TEXTURE_2D);
    for (int i = 0; i < 6; ++i) {
        skyboxTex[i] = LoadBitmap(faces[i]);
        if (skyboxTex[i] == (GLuint)-1) {
            MessageBox(NULL, faces[i], "Load failed", MB_OK);
            exit(0);
        }
        glBindTexture(GL_TEXTURE_2D, skyboxTex[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

// --- 繪製 Skybox ---
void drawSkybox(float size) {
    float s = size * 0.5f;
    glDepthMask(GL_FALSE);
    glPushMatrix();

    // +X 面
    glBindTexture(GL_TEXTURE_2D, skyboxTex[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 0); glVertex3f(s, -s, -s);
    glTexCoord2f(1, 1); glVertex3f(s, s, -s);
    glTexCoord2f(0, 1); glVertex3f(s, s, s);
    glTexCoord2f(0, 0); glVertex3f(s, -s, s);
    glEnd();

    // -X 面
    glBindTexture(GL_TEXTURE_2D, skyboxTex[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-s, -s, -s);
    glTexCoord2f(1, 0); glVertex3f(-s, -s, s);
    glTexCoord2f(1, 1); glVertex3f(-s, s, s);
    glTexCoord2f(0, 1); glVertex3f(-s, s, -s);
    glEnd();

    // +Y 面
    glBindTexture(GL_TEXTURE_2D, skyboxTex[2]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(-s, s, -s);
    glTexCoord2f(0, 0); glVertex3f(-s, s, s);
    glTexCoord2f(1, 0); glVertex3f(s, s, s);
    glTexCoord2f(1, 1); glVertex3f(s, s, -s);
    glEnd();

    // -Y 面
    glBindTexture(GL_TEXTURE_2D, skyboxTex[3]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-s, -s, -s);
    glTexCoord2f(1, 0); glVertex3f(s, -s, -s);
    glTexCoord2f(1, 1); glVertex3f(s, -s, s);
    glTexCoord2f(0, 1); glVertex3f(-s, -s, s);
    glEnd();

    // +Z 面
    glBindTexture(GL_TEXTURE_2D, skyboxTex[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-s, -s, s);
    glTexCoord2f(0, 1); glVertex3f(-s, s, s);
    glTexCoord2f(1, 1); glVertex3f(s, s, s);
    glTexCoord2f(1, 0); glVertex3f(s, -s, s);
    glEnd();

    // -Z 面
    glBindTexture(GL_TEXTURE_2D, skyboxTex[5]);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 0); glVertex3f(-s, -s, -s);
    glTexCoord2f(0, 0); glVertex3f(s, -s, -s);
    glTexCoord2f(0, 1); glVertex3f(s, s, -s);
    glTexCoord2f(1, 1); glVertex3f(-s, s, -s);
    glEnd();

    glPopMatrix();
    glDepthMask(GL_TRUE);
}

// --- 初始化障礙物 ---
void initObstacles() {
    obstacles.clear();
    srand((unsigned)time(nullptr));
    for (int i = 0; i < NUM_OBS; ++i) {
        Obstacle o;
        o.x = (rand() % 1800 - 900) * 0.5f;
        o.y = (rand() % 2400 - 1200) * 0.5f;
        o.z = posZ - (rand() % 200 + (int)SPAWN_DISTANCE);
        o.scale = ((rand() % 2) == 0) ? 5.5f : 2.5f;
        o.alive = true;
        obstacles.push_back(o);
    }
}
void initHealthPowerups() {
    healthPowerups.clear();
    srand((unsigned)time(nullptr) + 12345); // 使用不同的種子避免與障礙物重疊
    for (int i = 0; i < NUM_HEALTH_POWERUPS; ++i) {
        HealthPowerup h;
        h.x = (rand() % 1600 - 800) * 0.5f;
        h.y = (rand() % 2000 - 1000) * 0.5f;
        h.z = posZ - (rand() % 300 + (int)HEALTH_SPAWN_DISTANCE);
        h.scale = 2.0f; 
        h.alive = true;
        h.rotationY = 0.0f;
        healthPowerups.push_back(h);
    }
}

// 新增的 updateHealthPowerups() 函式：
void updateHealthPowerups() {
    for (size_t i = 0; i < healthPowerups.size(); ++i) {
        HealthPowerup& h = healthPowerups[i];
        if (h.alive) {
            h.z += OB_SPEED * 0.8f; // 移動速度稍慢一點
            h.rotationY += 3.0f; // 旋轉動畫
            if (h.rotationY > 360.0f) h.rotationY -= 360.0f;

            if (h.z > posZ + HEALTH_RESET_DISTANCE) {
                h.alive = false;
            }
        }
        else {
            h.z = posZ - (rand() % 300 + (int)HEALTH_SPAWN_DISTANCE);
            h.x = (rand() % 1600 - 800) * 0.5f;
            h.y = (rand() % 2000 - 1000) * 0.5f;
            h.scale = 2.0f;
            h.alive = true;
            h.rotationY = 0.0f;
        }
    }
}

// 新增的 checkHealthPowerupCollision() 函式：
void checkHealthPowerupCollision() {
    for (size_t i = 0; i < healthPowerups.size(); ++i) {
        HealthPowerup& h = healthPowerups[i];
        if (!h.alive) continue;

        float dx = posX - h.x;
        float dy = posY - h.y;
        float dz = posZ - h.z;
        float dist2 = dx * dx + dy * dy + dz * dz;
        float powerupRadius = h.scale * 2.0f;
        float r = shipRadius + powerupRadius;

        if (dist2 < r * r) {
            // 加血量，但不超過3
            if (health < 3) {
                health++;
            }
            h.alive = false;
            return;
        }
    }
}

// --- 初始化遊戲 ---
void init(void) {
    glClearColor(0.0, 0.0, 0.2, 0.0);
    glShadeModel(GL_SMOOTH);

    glViewport(0, 0, screen_width, screen_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)screen_width / (GLfloat)screen_height, 1.0f, 1000.0f);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_TEXTURE_2D);

    // 啟用 Alpha 混合用於透明度
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initSkybox();
    Load3DS(&object, "spaceship.3ds");
    Load3DS(&object1, "alian.3ds");
    Load3DS(&healthObject, "panda.3ds"); // 使用太空船模型作為血量道具（你可以換成其他模型）
    object.id_texture = LoadBitmap("spaceshiptexture.bmp");
    object1.id_texture = LoadBitmap("alian.bmp");
    healthObject.id_texture = LoadBitmap("health.bmp"); // 可以使用不同的紋理
    initObstacles();
    initHealthPowerups();

    playBackgroundMusic();
}

// --- 視窗大小改變時的處理 ---
void resize(int width, int height) {
    screen_width = width;
    screen_height = height;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, screen_width, screen_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)screen_width / (GLfloat)screen_height, 1.0f, 1000.0f);
    glutPostRedisplay();
}

// --- 更新障礙物位置 ---
void updateObstacles() {
    for (size_t i = 0; i < obstacles.size(); ++i) {
        Obstacle& o = obstacles[i];
        if (o.alive) {
            o.z += OB_SPEED;
            if (o.z > posZ + RESET_DISTANCE) {
                o.alive = false;
            }
        }
        else {
            o.z = posZ - (rand() % 200 + (int)SPAWN_DISTANCE);
            o.x = (rand() % 1800 - 900) * 0.5f;
            o.y = (rand() % 2400 - 1200) * 0.5f;
            o.scale = ((rand() % 2) == 0) ? 5.5f : 2.5f;
            o.alive = true;
        }
    }
}

// --- 碰撞檢測 ---
void checkCollision() {
    float maxDist2 = 0;
    for (int i = 0; i < object.polygons_qty; ++i) {
        auto& v = object.vertex[i];
        float d2 = v.x * v.x + v.y * v.y + v.z * v.z;
        maxDist2 = max(maxDist2, d2);
    }
    baseRadius = sqrt(maxDist2);
    shipRadius = baseRadius * 0.2f;

    for (size_t i = 0; i < obstacles.size(); ++i) {
        Obstacle& o = obstacles[i];
        if (!o.alive) continue;

        float dx = posX - o.x;
        float dy = posY - o.y;
        float dz = posZ - o.z;
        float dist2 = dx * dx + dy * dy + dz * dz;
        float obstacleRadius = o.scale * 1.f;
        float r = shipRadius + obstacleRadius;

        if (dist2 < r * r) {
            int deduction = (o.scale > 4.0f ? 3 : 1);
            score -= deduction;
            health--;

            // 觸發閃爍效果
            isFlickering = true;
            flickerTimer = 30; // 閃爍30幀
            if (health <= 0) {
                gameState = STATE_END;
            }
            o.alive = false;
            return;
        }
    }
}

// --- 鍵盤輸入處理 ---
void keyboard(unsigned char key, int x, int y) {
    if (gameState == STATE_START && key == ' ') {
        gameState = STATE_PLAYING;
        timeLeft = 25;
        health = 3;
        score = 0;
        gameTime = 0;
        isFlickering = false;
        flickerTimer = 0;
        initObstacles();
        initHealthPowerups(); 
        return;
    }
    if (gameState == STATE_END && key == 'r' || key == 'R') {
        gameState = STATE_START;
        posX = 0, posY = 0, posZ = 0;
        damping = 0.0f;
        angle_s = 0;
        angle_sx = 0;
        return;
    }
    if (gameState != STATE_PLAYING) return;

    switch (key) {
    case 'A':case 'a':
        posX -= 3.5;
        if (angle_s < 35) angle_s += 1.5;
        strcpy(lastKeyStr, "A");
        break;
    case 'D':case 'd':
        posX += 3.5;
        if (angle_s > -35) angle_s -= 1.5;
        strcpy(lastKeyStr, "D");
        break;
    case 'Z':case 'z': posZ -= 1.5; strcpy(lastKeyStr, "Z"); break;
    case 'X':case 'x': posZ += 1.5; strcpy(lastKeyStr, "X"); break;
    case 'W':case 'w':
        posY += 3.5;
        if (angle_sx < 15) angle_sx += 1.5;
        strcpy(lastKeyStr, "W");
        break;
    case 'S':case 's':
        posY -= 3.5;
        if (angle_sx > -15) angle_sx -= 1.5;
        strcpy(lastKeyStr, "S");
        break;
    case 27: exit(0); break;
    default:
        snprintf(lastKeyStr, sizeof(lastKeyStr), "%c", key);
        break;
    }
    glutPostRedisplay();
}

// --- 繪製字串函數 (改為亮白色並增加對比度) ---
void drawString(void* font, const char* str, int x, int y, float r = 1.0f, float g = 1.0f, float b = 1.0f, float alpha = 1.0f) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, screen_width, 0, screen_height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 禁用紋理以確保文字顏色正確顯示
    glDisable(GL_TEXTURE_2D);

    // 設定文字顏色，包含 alpha 值
    glColor4f(r, g, b, alpha);
    glRasterPos2i(x, y);
    while (*str) {
        glutBitmapCharacter(font, *str++);
    }

    // 重新啟用紋理
    glEnable(GL_TEXTURE_2D);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// --- 繪製 3D 物件 ---
void drawObject(obj_type2 obj) {
    glBindTexture(GL_TEXTURE_2D, obj.id_texture);
    glBegin(GL_TRIANGLES);
    for (int l_index = 0; l_index < obj.polygons_qty; l_index++) {
        glTexCoord2f(obj.mapcoord[obj.polygon[l_index].a].u,
            obj.mapcoord[obj.polygon[l_index].a].v);
        glVertex3f(obj.vertex[obj.polygon[l_index].a].x,
            obj.vertex[obj.polygon[l_index].a].y,
            obj.vertex[obj.polygon[l_index].a].z);

        glTexCoord2f(obj.mapcoord[obj.polygon[l_index].b].u,
            obj.mapcoord[obj.polygon[l_index].b].v);
        glVertex3f(obj.vertex[obj.polygon[l_index].b].x,
            obj.vertex[obj.polygon[l_index].b].y,
            obj.vertex[obj.polygon[l_index].b].z);

        glTexCoord2f(obj.mapcoord[obj.polygon[l_index].c].u,
            obj.mapcoord[obj.polygon[l_index].c].v);
        glVertex3f(obj.vertex[obj.polygon[l_index].c].x,
            obj.vertex[obj.polygon[l_index].c].y,
            obj.vertex[obj.polygon[l_index].c].z);
    }
    glEnd();
}

// --- 限制值範圍函數 ---
template <typename T>
T clamp(T value, T minVal, T maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

// --- 計時器函數 ---
void timerFunc(int) {
    if (gameState == STATE_PLAYING) {
        timeLeft--;
        gameTime++;

        // 即時加分系統 - 每秒存活加2.5分
        score += 2;

        if (timeLeft <= 0) {
            // 時間到，根據血量額外加分
            if (health == 3) score += 50;
            else if (health == 2) score += 40;
            else if (health == 1) score += 30;

            gameState = STATE_END;
        }

        if (health <= 0) {
            gameState = STATE_END;
        }
    }
    glutTimerFunc(1000, timerFunc, 0);
}

// --- 繪製開始畫面 (調整排版並改為英文) ---
void drawStartScreen() {
    // 繪製一個暗色半透明背景，讓文字更突出
    // 使用漸變效果，讓畫面更有層次感
    glBegin(GL_QUADS);
    glColor4f(0.0f, 0.0f, 0.2f, 0.8f); glVertex2f(0, 0); // 較深的藍色，透明度更高
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f); glVertex2f(screen_width, 0);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f); glVertex2f(screen_width, screen_height);
    glColor4f(0.0f, 0.0f, 0.2f, 0.8f); glVertex2f(0, screen_height);
    glEnd();

    // 遊戲標題
    drawString(GLUT_BITMAP_TIMES_ROMAN_24, "3D Space Adventure", screen_width / 2 - 95, screen_height / 2 + 80, 0.8f, 1.0f, 1.0f); // 淺藍色

    // 主要操作提示
    drawString(GLUT_BITMAP_TIMES_ROMAN_24, "Press SPACE to Start", screen_width / 2 - 95, screen_height / 2 + 40, 1.0f, 1.0f, 0.8f); // 淺黃色

    // 操作說明
    drawString(GLUT_BITMAP_HELVETICA_18, "Goal: Avoid obstacles and survive!", screen_width / 2 - 125, screen_height / 2 - 0, 1.0f, 1.0f, 1.0f);
    drawString(GLUT_BITMAP_HELVETICA_18, "Use WASDZX to Move", screen_width / 2 - 125, screen_height / 2 - 30, 1.0f, 1.0f, 1.0f);
    drawString(GLUT_BITMAP_HELVETICA_18, "Collect hearts to restore health", screen_width / 2 - 125, screen_height / 2 - 60, 1.0f, 1.0f, 1.0f);
    drawString(GLUT_BITMAP_HELVETICA_18, "Press ESC to exit game", screen_width / 2 - 125, screen_height / 2 - 90, 1.0f, 1.0f, 1.0f);
}

// --- 繪製結束畫面 (調整排版並改為英文) ---
void drawEndScreen() {
    // 繪製一個暗色半透明背景
    glBegin(GL_QUADS);
    glColor4f(0.2f, 0.0f, 0.0f, 0.8f); glVertex2f(0, 0); // 較深的紅色，透明度更高
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f); glVertex2f(screen_width, 0);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f); glVertex2f(screen_width, screen_height);
    glColor4f(0.2f, 0.0f, 0.0f, 0.8f); glVertex2f(0, screen_height);
    glEnd();

    char buf[64];
    sprintf(buf, "Game Over! Final Score: %.1f", score);
    drawString(GLUT_BITMAP_TIMES_ROMAN_24, buf, screen_width / 2 - 120, screen_height / 2 + 20, 1.0f, 0.8f, 0.8f); // 淺紅色
    drawString(GLUT_BITMAP_TIMES_ROMAN_24, "Press R to Restart", screen_width / 2 - 80, screen_height / 2 - 20, 1.0f, 1.0f, 0.8f); // 淺黃色
}

// --- 繪製遊戲場景 ---
void drawGameScene() {
    // 限制太空船移動範圍
    posX = clamp(posX, -600.0, 600.0);
    posY = clamp(posY, -700.0, 700.0);
    posZ = clamp(posZ, -200.0, 300.0);

    // 設定攝影機位置
    double camYOffset = 0.0;
    double camZOffset = 130.0;
    gluLookAt(posX, posY + camYOffset, posZ + camZOffset,
        posX, posY, posZ, 0, 1, 0);

    // 畫 Skybox
    glDepthMask(GL_FALSE);
    glPushMatrix();
    glTranslatef(0, 0, 250); // Skybox 應固定於攝影機視點
    drawSkybox(1600.0f);  // 增大skybox確保不會破圖
    glPopMatrix();
    glDepthMask(GL_TRUE);

    // 顯示血量 (改為亮白色文字，位置更顯眼)
    char healthStr[32];
    sprintf(healthStr, "Health: ");
    for (int i = 0; i < health; ++i) {
        strcat(healthStr, "<3 "); // 使用可愛的心形符號或自定義圖示
    }
    drawString(GLUT_BITMAP_TIMES_ROMAN_24, healthStr, 20, screen_height - 30, 1.0f, 0.5f, 0.5f); // 紅色

    // 顯示剩餘時間 (亮白色文字，位置更顯眼)
    char timeBuffer[32];
    sprintf(timeBuffer, "Time: %02d", timeLeft);
    // 時間低於5秒時閃爍變紅
    if (timeLeft <= 5 && (gameTime % 2 == 0)) { // 每秒閃爍一次
        drawString(GLUT_BITMAP_TIMES_ROMAN_24, timeBuffer, screen_width - 150, screen_height - 30, 1.0f, 0.0f, 0.0f); // 紅色
    }
    else {
        drawString(GLUT_BITMAP_TIMES_ROMAN_24, timeBuffer, screen_width - 150, screen_height - 30, 1.0f, 1.0f, 0.8f); // 淺黃色
    }

    // 顯示分數 (亮白色文字，即時更新，並有動畫)
    char scoreBuffer[64];
    sprintf(scoreBuffer, "Score: %.1f", score);
    drawString(GLUT_BITMAP_TIMES_ROMAN_24, scoreBuffer, screen_width / 2 - 50, screen_height - 30, 1.0f, 1.0f, 1.0f);

    // 更新閃爍效果
    if (isFlickering) {
        flickerTimer--;
        if (flickerTimer <= 0) {
            isFlickering = false;
        }
    }

    // 畫太空船
    bool shouldDrawShip = true;
    if (isFlickering && (flickerTimer % 4 < 2)) { // 閃爍時更規律地顯示/隱藏
        shouldDrawShip = false;
    }

    if (shouldDrawShip) {
        glPushMatrix();

        // 設定振動幅度
        float dampingStrength = isFlickering ? collisionDamping : normalDamping;
        damping = (float)rand() / (float)(RAND_MAX)*dampingStrength;

        glTranslatef(posX + damping, posY + damping, posZ + damping);
        glRotatef(angle_sx, 1.0, 0.0, 0);
        glRotatef(angle_s, 0.0, 0.0, 1.0);
        glRotatef(180, 0.0, 1.0, 0);
        glRotatef(270, 1.0, 0.0, 0);
        glScalef(0.2, 0.2, 0.2);
        drawObject(object);
        glPopMatrix();
    }

    // 畫障礙物
    for (size_t i = 0; i < obstacles.size(); ++i) {
        Obstacle& o = obstacles[i];
        if (!o.alive) continue;

        glPushMatrix();
        glTranslatef(o.x, o.y, o.z);
        glRotatef(115, 1.0, 0, 0);
        glRotatef(180, 0.0, 1.0, 0);
        glRotatef(180, 1.0, 0.0, 0);
        glRotatef(180, 0.0, 1.0, 0.0);
        glScalef(o.scale, o.scale, o.scale);
        drawObject(object1);
        glPopMatrix();
    }

    updateObstacles();
    checkCollision();
    // 畫血量道具
    for (size_t i = 0; i < healthPowerups.size(); ++i) {
        HealthPowerup& h = healthPowerups[i];
        if (!h.alive) continue;

        glPushMatrix();
        glTranslatef(h.x, h.y, h.z);
        glRotatef(h.rotationY, 0.0, 1.0, 0.0); // Y軸旋轉
        glRotatef(45, 1.0, 0, 0); // 稍微傾斜
        glRotatef(180, 1.0, 0.0, 0.0); // Y軸旋轉
        glScalef(h.scale, h.scale, h.scale);
        drawObject(healthObject);
        glPopMatrix();
    }

    updateObstacles();
    updateHealthPowerups(); 
    checkCollision();
    checkHealthPowerupCollision(); 
}

// --- 顯示函數 ---
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    switch (gameState) {
    case STATE_START:
        drawStartScreen();
        break;
    case STATE_PLAYING:
        drawGameScene();
        break;
    case STATE_END:
        drawEndScreen();
        break;
    }

    glFlush();
    glutSwapBuffers();
}

// --- 閒置函數 ---
void idle() {
    const double speed = 0.002;
    camAngle += speed;
    if (camAngle > 2 * M_PI)
        camAngle -= 2 * M_PI;

    if (gameState == STATE_PLAYING) {
        updateObstacles();
        updateHealthPowerups(); 
        checkCollision();
        checkHealthPowerupCollision(); 
    }
    glutPostRedisplay();
}

// --- 主函數 ---
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screen_width, screen_height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("3D Space Adventure"); // 視窗標題也改為英文

    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);

    init();
    glutTimerFunc(1000, timerFunc, 0);
    glutMainLoop();
    stopBackgroundMusic();
    return 0;
}