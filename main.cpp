#ifdef __APPLE__
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159f

/* -- Game States -- */
#define STATE_START 0
#define STATE_PLAY  1
#define STATE_PAUSE 2
#define STATE_OVER  3

/* -- Global Game Variables -- */
int   gameState = STATE_START;
int   score     = 0;
int   lives     = 3;
int   level     = 1;

/* -- Lane X positions -- */
float laneX[3] = { -12.0f, 0.0f, 12.0f };

/* ================================================================
   MEMBER 1 - BACKGROUND
   STATUS: COMPLETE
   Sky gradient, road, DDA lane markings, clouds, trees
   ================================================================ */

float dashOffset = 0.0f;

typedef struct { float x, y, scale, speed; } Cloud;
Cloud clouds[4];

void initBackground(void)
{
    clouds[0].x=-25.0f; clouds[0].y=32.0f; clouds[0].scale=1.0f;  clouds[0].speed=0.02f;
    clouds[1].x= 10.0f; clouds[1].y=36.0f; clouds[1].scale=0.7f;  clouds[1].speed=0.03f;
    clouds[2].x= 25.0f; clouds[2].y=30.0f; clouds[2].scale=1.2f;  clouds[2].speed=0.015f;
    clouds[3].x=-10.0f; clouds[3].y=34.0f; clouds[3].scale=0.9f;  clouds[3].speed=0.025f;
    dashOffset = 0.0f;
}

/*
 *  DDA LINE ALGORITHM  (EP5 - Course Requirement)
 *  Computes dx and dy, takes steps = max(|dx|,|dy|),
 *  increments x by dx/steps and y by dy/steps each step.
 *  Used to draw the scrolling yellow lane divider dashes.
 */
void ddaLine(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1, dy = y2 - y1;
    float adx = dx < 0 ? -dx : dx;
    float ady = dy < 0 ? -dy : dy;
    int steps = (int)(adx > ady ? adx : ady);
    if (steps == 0) return;
    float xi = dx / steps, yi = dy / steps;
    float x = x1, y = y1;
    int i;
    glBegin(GL_POINTS);
    for (i = 0; i <= steps; i++) { glVertex2f(x, y); x += xi; y += yi; }
    glEnd();
}

void drawSky(void)
{
    /* Gradient sky: light blue at horizon, deep blue at top */
    glBegin(GL_QUADS);
        glColor3f(0.40f, 0.70f, 0.95f); glVertex2f(-35,-5); glVertex2f(35,-5);
        glColor3f(0.10f, 0.28f, 0.70f); glVertex2f(35,40);  glVertex2f(-35,40);
    glEnd();
    /* Green grassy shoulders on both sides of road */
    glColor3f(0.20f, 0.55f, 0.15f);
    glBegin(GL_QUADS);
        glVertex2f(-35,-40); glVertex2f(-18,-40); glVertex2f(-18,40); glVertex2f(-35,40);
    glEnd();
    glBegin(GL_QUADS);
        glVertex2f(18,-40);  glVertex2f(35,-40);  glVertex2f(35,40);  glVertex2f(18,40);
    glEnd();
}

void drawRoad(void)
{
    /* Dark asphalt rectangle */
    glColor3f(0.22f, 0.22f, 0.25f);
    glBegin(GL_QUADS);
        glVertex2f(-18,-40); glVertex2f(18,-40); glVertex2f(18,40); glVertex2f(-18,40);
    glEnd();
    /* White edge lines using GL_LINES */
    glColor3f(0.95f, 0.95f, 0.95f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
        glVertex2f(-18,-40); glVertex2f(-18,40);
        glVertex2f( 18,-40); glVertex2f( 18,40);
    glEnd();
    glLineWidth(1.0f);
}

void drawRoadMarkings(void)
{
    /*
     *  Lane dividers at x=-6 and x=+6 drawn using DDA algorithm (EP5).
     *  dashOffset scrolls each frame, making dashes appear to move downward
     *  and simulating forward driving motion.
     */
    float lx[2] = { -6.0f, 6.0f };
    float dashLen = 6.0f, period = 9.0f;
    int lane;
    glColor3f(1.0f, 0.90f, 0.0f);
    glPointSize(2.5f);
    for (lane = 0; lane < 2; lane++) {
        float sy = -40.0f - dashOffset;
        while (sy < 40.0f + period) {
            float y1 = sy, y2 = sy + dashLen;
            if (y1 > 40.0f) break;
            if (y2 < -40.0f) { sy += period; continue; }
            if (y1 < -40.0f) y1 = -40.0f;
            if (y2 > 40.0f)  y2 =  40.0f;
            ddaLine(lx[lane], y1, lx[lane], y2);
            sy += period;
        }
    }
    glPointSize(1.0f);
}

void updateRoadMarkings(void)
{
    /* Advance dashOffset each frame; wrap at period to create infinite scroll */
    dashOffset += 0.40f;
    if (dashOffset >= 9.0f) dashOffset -= 9.0f;
}

void drawOneCloud(float cx, float cy, float sc)
{
    /* Cloud built from 4 overlapping GL_POLYGON circles at offset positions */
    float ox[4]={0,2.5f,-2.5f,0}, oy[4]={0,0.5f,0.5f,1.5f}, r[4]={2.5f,2,2,2.2f};
    int b, i;
    glColor3f(0.98f, 0.98f, 1.0f);
    for (b = 0; b < 4; b++) {
        glBegin(GL_POLYGON);
        for (i = 0; i < 30; i++) {
            float a = 2*PI*i/30;
            glVertex2f(cx+ox[b]*sc + r[b]*sc*(float)cos(a),
                       cy+oy[b]*sc + r[b]*sc*(float)sin(a));
        }
        glEnd();
    }
}

void drawClouds(void)
{
    int i;
    for (i = 0; i < 4; i++) drawOneCloud(clouds[i].x, clouds[i].y, clouds[i].scale);
}

void updateClouds(void)
{
    /* Translate each cloud rightward; wrap to left edge when it exits right */
    int i;
    for (i = 0; i < 4; i++) {
        clouds[i].x += clouds[i].speed;
        if (clouds[i].x > 42.0f) clouds[i].x = -42.0f;
    }
}

void drawOneTree(float tx, float ty)
{
    /* Brown trunk (GL_QUADS) + 3 stacked green triangle layers (GL_TRIANGLES) */
    glColor3f(0.36f,0.20f,0.08f);
    glBegin(GL_QUADS);
        glVertex2f(tx-0.6f,ty-4); glVertex2f(tx+0.6f,ty-4);
        glVertex2f(tx+0.6f,ty);   glVertex2f(tx-0.6f,ty);
    glEnd();
    glColor3f(0.10f,0.48f,0.10f);
    glBegin(GL_TRIANGLES); glVertex2f(tx-3.5f,ty); glVertex2f(tx+3.5f,ty); glVertex2f(tx,ty+6); glEnd();
    glColor3f(0.13f,0.58f,0.13f);
    glBegin(GL_TRIANGLES); glVertex2f(tx-2.8f,ty+2.5f); glVertex2f(tx+2.8f,ty+2.5f); glVertex2f(tx,ty+8); glEnd();
    glColor3f(0.16f,0.68f,0.16f);
    glBegin(GL_TRIANGLES); glVertex2f(tx-2.0f,ty+5); glVertex2f(tx+2.0f,ty+5); glVertex2f(tx,ty+10); glEnd();
}

void drawSideTrees(void)
{
    float lx[3]={-22,-28,-32}, rx[3]={22,28,32}, y[3]={-25,5,30};
    int i;
    for (i=0;i<3;i++) { drawOneTree(lx[i],y[i]); drawOneTree(rx[i],y[i]); }
}

/* ================================================================
   MEMBER 2 - PLAYER CAR
   STATUS: COMPLETE
   Car body polygons, Midpoint Circle wheels, smooth lane movement
   ================================================================ */

float playerX = 0.0f, playerY = -28.0f, targetX = 0.0f, playerTilt = 0.0f;
int   currentLane = 1;

/* Utility: filled circle using GL_POLYGON with cos/sin vertices */
void filledCircle(float cx, float cy, float r, int seg)
{
    int i;
    glBegin(GL_POLYGON);
    for (i=0;i<seg;i++) {
        float a=2*PI*i/seg;
        glVertex2f(cx+r*(float)cos(a), cy+r*(float)sin(a));
    }
    glEnd();
}

/*
 *  MIDPOINT CIRCLE ALGORITHM  (EP5 - Course Requirement)
 *  Starts at (0, r) with decision parameter d = 1 - r.
 *  Each step: plots 8 symmetric points (one per octant).
 *  If d < 0: d += 2*xi + 3. Else: d += 2*(xi-yi) + 5, yi--.
 *  Used to draw the tyre outline on each wheel.
 */
void midpointCircleOutline(float cx, float cy, float r)
{
    int xi=0, yi=(int)r, d=1-(int)r;
    float scale = (yi > 0) ? r/yi : 1.0f;
    glBegin(GL_POINTS);
    while (xi <= yi) {
        float rx=(float)xi*scale, ry=(float)yi*scale;
        glVertex2f(cx+rx,cy+ry); glVertex2f(cx-rx,cy+ry);
        glVertex2f(cx+rx,cy-ry); glVertex2f(cx-rx,cy-ry);
        glVertex2f(cx+ry,cy+rx); glVertex2f(cx-ry,cy+rx);
        glVertex2f(cx+ry,cy-rx); glVertex2f(cx-ry,cy-rx);
        if (d<0) d+=2*xi+3; else { d+=2*(xi-yi)+5; yi--; }
        xi++;
    }
    glEnd();
}

void drawWheel(float wx, float wy)
{
    int sp;
    float r=0.95f;
    /* Tyre: black filled circle */
    glColor3f(0.08f,0.08f,0.08f); filledCircle(wx,wy,r,32);
    /* Midpoint circle outline on tyre edge (EP5 algorithm visible here) */
    glColor3f(0.30f,0.30f,0.35f); glPointSize(1.5f); midpointCircleOutline(wx,wy,r*0.97f); glPointSize(1.0f);
    /* Rim, hub, spokes */
    glColor3f(0.72f,0.72f,0.80f); filledCircle(wx,wy,r*0.55f,20);
    glColor3f(0.92f,0.92f,0.96f); filledCircle(wx,wy,r*0.20f,12);
    glColor3f(0.60f,0.60f,0.65f); glLineWidth(1.5f);
    for (sp=0;sp<5;sp++) {
        float a=2*PI*sp/5;
        glBegin(GL_LINES); glVertex2f(wx,wy); glVertex2f(wx+r*0.50f*(float)cos(a),wy+r*0.50f*(float)sin(a)); glEnd();
    }
    glLineWidth(1.0f);
}

void drawPlayerCar(float px, float py)
{
    float hw=2.25f, hh=3.50f;
    glPushMatrix();
    glTranslatef(px,py,0);           /* Translation: moves car to world position (EP5) */
    glRotatef(playerTilt,0,0,1);     /* Rotation: lane-change tilt around Z-axis (EP5) */

    glColor3f(0.88f,0.12f,0.12f);   /* Main body - red */
    glBegin(GL_QUADS); glVertex2f(-hw,-hh); glVertex2f(hw,-hh); glVertex2f(hw,hh*0.5f); glVertex2f(-hw,hh*0.5f); glEnd();

    glColor3f(0.65f,0.08f,0.08f);   /* Cabin / roof */
    glBegin(GL_QUADS); glVertex2f(-hw*0.72f,hh*0.5f); glVertex2f(hw*0.72f,hh*0.5f); glVertex2f(hw*0.55f,hh); glVertex2f(-hw*0.55f,hh); glEnd();

    glColor3f(0.28f,0.52f,0.90f);   /* Front windshield */
    glBegin(GL_QUADS); glVertex2f(-hw*0.54f,hh*0.5f); glVertex2f(hw*0.54f,hh*0.5f); glVertex2f(hw*0.40f,hh*0.93f); glVertex2f(-hw*0.40f,hh*0.93f); glEnd();

    glColor3f(0.22f,0.45f,0.80f);   /* Rear window */
    glBegin(GL_QUADS); glVertex2f(-hw*0.50f,-hh*0.05f); glVertex2f(hw*0.50f,-hh*0.05f); glVertex2f(hw*0.62f,hh*0.44f); glVertex2f(-hw*0.62f,hh*0.44f); glEnd();

    /* Headlights (two circles, outer + inner glow) */
    glColor3f(1,1,0.65f); filledCircle(-hw*0.62f,hh*0.5f,0.42f,16); filledCircle(hw*0.62f,hh*0.5f,0.42f,16);
    glColor3f(1,1,0.35f); filledCircle(-hw*0.62f,hh*0.5f,0.22f,16); filledCircle(hw*0.62f,hh*0.5f,0.22f,16);

    /* Tail lights */
    glColor3f(0.95f,0.08f,0.08f);
    glBegin(GL_QUADS); glVertex2f(-hw,-hh); glVertex2f(-hw*0.6f,-hh); glVertex2f(-hw*0.6f,-hh+0.55f); glVertex2f(-hw,-hh+0.55f); glEnd();
    glBegin(GL_QUADS); glVertex2f(hw*0.6f,-hh); glVertex2f(hw,-hh); glVertex2f(hw,-hh+0.55f); glVertex2f(hw*0.6f,-hh+0.55f); glEnd();

    /* Side stripe */
    glColor3f(0.18f,0.18f,0.18f);
    glBegin(GL_QUADS); glVertex2f(-hw,-hh*0.22f); glVertex2f(hw,-hh*0.22f); glVertex2f(hw,-hh*0.06f); glVertex2f(-hw,-hh*0.06f); glEnd();

    /* Four wheels */
    drawWheel(-hw-0.25f, hh*0.12f); drawWheel(hw+0.25f, hh*0.12f);
    drawWheel(-hw-0.25f,-hh*0.50f); drawWheel(hw+0.25f,-hh*0.50f);
    glPopMatrix();
}

void initPlayerCar(void)
{
    currentLane=1; playerX=laneX[1]; targetX=laneX[1]; playerTilt=0;
}

void movePlayerLeft(void)  { if(currentLane>0){ currentLane--; targetX=laneX[currentLane]; playerTilt= 6.0f; } }
void movePlayerRight(void) { if(currentLane<2){ currentLane++; targetX=laneX[currentLane]; playerTilt=-6.0f; } }

void updatePlayerCar(void)
{
    /* Smooth lerp: playerX moves 18% of remaining distance toward targetX each frame */
    float d=targetX-playerX;
    if(d>0.05f||d<-0.05f) playerX+=d*0.18f;
    else { playerX=targetX; playerTilt*=0.85f; if(playerTilt>-0.1f&&playerTilt<0.1f) playerTilt=0; }
}

/* ================================================================
   MEMBER 3 - ENEMY CARS. Moons Part yahooooooooooo
   STATUS: COMPLETE
   Three opponents per lane, Bresenham outlines, level-based speed
   ================================================================ */

typedef struct { float x,y,speed,r,g,b; int lane,active; } EnemyCar;
EnemyCar enemies[3];

/*
 *  BRESENHAM LINE ALGORITHM  (EP5 - Course Requirement)
 *  Uses integer error term (err = dx - dy) to decide each step.
 *  If e2 > -dy: advance in x. If e2 < dx: advance in y.
 *  No floating-point division � pure integer arithmetic.
 *  Used to draw the 4 edge outlines of each enemy car body.
 */
void bresenhamLine(int x1,int y1,int x2,int y2)
{
    int dx=abs(x2-x1), dy=abs(y2-y1), sx=(x1<x2)?1:-1, sy=(y1<y2)?1:-1, err=dx-dy, e2;
    glBegin(GL_POINTS);
    for(;;){
        glVertex2i(x1,y1);
        if(x1==x2&&y1==y2) break;
        e2=2*err;
        if(e2>-dy){err-=dy;x1+=sx;}
        if(e2< dx){err+=dx;y1+=sy;}
    }
    glEnd();
}

/* Floating-point wrapper: scale world coords * 80 to get integer grid */
void bresenhamF(float x1,float y1,float x2,float y2)
{
    int S=80;
    bresenhamLine((int)(x1*S),(int)(y1*S),(int)(x2*S),(int)(y2*S));
}

void drawEnemyCar(float ex,float ey,float cr,float cg,float cb)
{
    float hw=2.0f, hh=3.25f;
    glPushMatrix();
    glTranslatef(ex,ey,0);    /* Translation: position each enemy independently (EP5) */

    /* Coloured body */
    glColor3f(cr,cg,cb);
    glBegin(GL_QUADS); glVertex2f(-hw,-hh); glVertex2f(hw,-hh); glVertex2f(hw,hh*0.5f); glVertex2f(-hw,hh*0.5f); glEnd();

    /* Darker roof */
    glColor3f(cr*0.75f,cg*0.75f,cb*0.75f);
    glBegin(GL_QUADS); glVertex2f(-hw*0.70f,hh*0.5f); glVertex2f(hw*0.70f,hh*0.5f); glVertex2f(hw*0.55f,hh); glVertex2f(-hw*0.55f,hh); glEnd();

    /* Windshield */
    glColor3f(0.28f,0.52f,0.90f);
    glBegin(GL_QUADS); glVertex2f(-hw*0.52f,hh*0.5f); glVertex2f(hw*0.52f,hh*0.5f); glVertex2f(hw*0.40f,hh*0.93f); glVertex2f(-hw*0.40f,hh*0.93f); glEnd();

    /* Headlights (at bottom because enemy faces downward) */
    glColor3f(1,1,0.55f); filledCircle(-hw*0.60f,-hh+0.45f,0.38f,16); filledCircle(hw*0.60f,-hh+0.45f,0.38f,16);

    /* Bresenham outlines on all 4 edges (EP5 algorithm visible here) */
    glColor3f(0,0,0); glPointSize(1.8f);
    bresenhamF(-hw,-hh, hw,-hh);   /* bottom edge */
    bresenhamF(-hw, hh, hw, hh);   /* top edge    */
    bresenhamF(-hw,-hh,-hw, hh);   /* left edge   */
    bresenhamF( hw,-hh, hw, hh);   /* right edge  */
    glPointSize(1.0f);

    /* Wheels */
    glColor3f(0.10f,0.10f,0.10f);
    filledCircle(-hw-0.18f, hh*0.12f,0.82f,20); filledCircle(hw+0.18f, hh*0.12f,0.82f,20);
    filledCircle(-hw-0.18f,-hh*0.48f,0.82f,20); filledCircle(hw+0.18f,-hh*0.48f,0.82f,20);
    glColor3f(0.65f,0.65f,0.70f);
    filledCircle(-hw-0.18f, hh*0.12f,0.42f,16); filledCircle(hw+0.18f, hh*0.12f,0.42f,16);
    filledCircle(-hw-0.18f,-hh*0.48f,0.42f,16); filledCircle(hw+0.18f,-hh*0.48f,0.42f,16);
    glPopMatrix();
}

void initEnemyCars(void)
{
    /* Three cars: blue (lane 0), green (lane 1), yellow (lane 2) */
    float col[3][3]={{0.15f,0.35f,0.90f},{0.10f,0.72f,0.20f},{0.95f,0.75f,0.10f}};
    int i;
    for(i=0;i<3;i++){
        enemies[i].lane=i; enemies[i].x=laneX[i];
        enemies[i].y=10.0f+i*22.0f;           /* staggered start heights */
        enemies[i].speed=0.30f+i*0.04f;
        enemies[i].r=col[i][0]; enemies[i].g=col[i][1]; enemies[i].b=col[i][2];
        enemies[i].active=1;
    }
}

void updateEnemyCars(int lvl)
{
    /* baseSpeed increases with level: 0.28 + lvl*0.07 */
    float base=0.28f+lvl*0.07f; int i;
    for(i=0;i<3;i++){
        if(!enemies[i].active) continue;
        enemies[i].y-=(enemies[i].speed+base);
        if(enemies[i].y<-50.0f){
            /* Respawn at top with random lane and new random speed */
            int nl=rand()%3;
            enemies[i].lane=nl; enemies[i].x=laneX[nl];
            enemies[i].y=50.0f+(float)(rand()%20);
            enemies[i].speed=0.18f+(float)(rand()%22)/100.0f;
        }
    }
}

void drawAllEnemyCars(void)
{
    int i;
    for(i=0;i<3;i++)
        if(enemies[i].active)
            drawEnemyCar(enemies[i].x,enemies[i].y,enemies[i].r,enemies[i].g,enemies[i].b);
}

/* ================================================================
   MEMBER 4 - HUD & SCREENS
   STATUS: ~70% COMPLETE
   DONE    : Score counter, lives hearts, level indicator, start screen,
             pause overlay, drawPanel transformations
   IN PROG : HUD panel alpha transparency needs fine-tuning on Mac
   PLANNED : Game Over screen visual polish (layout drafted, not final)
   ================================================================ */

void renderText(float x,float y,const char* t,void* font)
{
    const char* c; glRasterPos2f(x,y); for(c=t;*c;c++) glutBitmapCharacter(font,*c);
}

/*
 *  drawPanel uses 2D Transformations (EP5):
 *  glTranslatef moves origin to panel corner position.
 *  glScalef stretches unit square to desired panel dimensions.
 *  This means one function handles any panel of any size anywhere.
 */
void drawPanel(float bx,float by,float bw,float bh,float r,float g,float b,float a)
{
    glPushMatrix();
    glTranslatef(bx,by,0);   /* Translation (EP5) */
    glScalef(bw,bh,1);       /* Scaling     (EP5) */
    glColor4f(r,g,b,a);
    glBegin(GL_QUADS); glVertex2f(0,0); glVertex2f(1,0); glVertex2f(1,1); glVertex2f(0,1); glEnd();
    glPopMatrix();
}

/* Heart icon: two GL_POLYGON semicircles + GL_TRIANGLES bottom point */
void drawHeart(float hx,float hy,float size)
{
    int i;
    glPushMatrix();
    glTranslatef(hx,hy,0); glScalef(size,size,1);   /* Translate + Scale (EP5) */
    glColor3f(1,0.12f,0.12f);
    glBegin(GL_POLYGON); for(i=0;i<=30;i++){ float a=PI*i/30; glVertex2f(-0.5f+0.5f*(float)cos(a),0.5f*(float)sin(a)); } glEnd();
    glBegin(GL_POLYGON); for(i=0;i<=30;i++){ float a=PI*i/30; glVertex2f(0.5f-0.5f*(float)cos(a),0.5f*(float)sin(a)); } glEnd();
    glBegin(GL_TRIANGLES); glVertex2f(-1,0); glVertex2f(1,0); glVertex2f(0,-1.2f); glEnd();
    glPopMatrix();
}

/* Score panel top-left */
void drawScore(int sc)
{
    char buf[64];
    drawPanel(-35,33.5f,20,5.5f,0,0,0,0.55f);   /* TODO: alpha 0.55 looks washed out on some Macs - IN PROG */
    glColor3f(1,1,0);
    sprintf(buf,"SCORE: %06d",sc);
    renderText(-34.2f,35.5f,buf,GLUT_BITMAP_HELVETICA_18);
}

/* Lives panel top-right */
void drawLives(int lv)
{
    int i;
    drawPanel(17.5f,33.5f,17.5f,5.5f,0,0,0,0.55f);
    glColor3f(1,1,1); renderText(18.5f,35.5f,"LIVES:",GLUT_BITMAP_HELVETICA_18);
    for(i=0;i<lv&&i<3;i++) drawHeart(27.0f+i*3.5f,36.0f,1.2f);
    /* TODO: heart spacing needs adjustment when lives=1 edge case - IN PROG */
}

/* Level indicator top-centre */
void drawLevel(int lvl)
{
    char buf[32];
    drawPanel(-5,33.5f,10,5.5f,0,0,0.22f,0.62f);
    glColor3f(0.4f,1,1);
    sprintf(buf,"LVL %d",lvl);
    renderText(-3.2f,35.5f,buf,GLUT_BITMAP_HELVETICA_18);
    /* TODO: level-up flash effect when level increases - PLANNED */
}

/* Pause overlay */
void drawPauseOverlay(void)
{
    glColor4f(0,0,0,0.48f);
    glBegin(GL_QUADS); glVertex2f(-35,-40); glVertex2f(35,-40); glVertex2f(35,40); glVertex2f(-35,40); glEnd();
    drawPanel(-12,-4.5f,24,9,0.05f,0.05f,0.18f,0.92f);
    glColor3f(1,1,0); renderText(-5.5f,2,"PAUSED",GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.88f,0.88f,0.88f); renderText(-10.5f,-2.5f,"Press SPACE to Resume",GLUT_BITMAP_HELVETICA_12);
}

/* Start screen - COMPLETE */
void drawStartScreen(void)
{
    int i;
    /* Dark background */
    glColor3f(0.04f,0.04f,0.10f);
    glBegin(GL_QUADS); glVertex2f(-35,-40); glVertex2f(35,-40); glVertex2f(35,40); glVertex2f(-35,40); glEnd();
    /* Decorative road strip */
    glColor3f(0.22f,0.22f,0.25f);
    glBegin(GL_QUADS); glVertex2f(-8,-40); glVertex2f(8,-40); glVertex2f(8,10); glVertex2f(-8,10); glEnd();
    glColor3f(1,0.9f,0);
    for(i=-5;i<3;i++){ glBegin(GL_QUADS); glVertex2f(-0.5f,i*10.0f); glVertex2f(0.5f,i*10.0f); glVertex2f(0.5f,i*10+5); glVertex2f(-0.5f,i*10+5); glEnd(); }
    /* Title panel */
    drawPanel(-20,10,40,14,0.08f,0.04f,0.28f,0.92f);
    glColor3f(1,0.85f,0);    renderText(-15.0f,21,"FORZA HORIZON 2099",GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.80f,0.88f,1); renderText(-17.5f,16.5f,"CSE422 Computer Graphics Lab",GLUT_BITMAP_HELVETICA_12);
    renderText(-10.5f,13.5f,"DIU  |  Spring 2026",GLUT_BITMAP_HELVETICA_12);
    /* Controls panel */
    drawPanel(-18,-16,36,22,0.05f,0.05f,0.15f,0.88f);
    glColor3f(0.4f,1,1); renderText(-6.5f,3.5f,"HOW TO PLAY",GLUT_BITMAP_HELVETICA_18);
    glColor3f(1,1,1);
    renderText(-16.5f,-0.0f,"LEFT / A   :  Move Left",GLUT_BITMAP_HELVETICA_12);
    renderText(-16.5f,-3.0f,"RIGHT / D  :  Move Right",GLUT_BITMAP_HELVETICA_12);
    renderText(-16.5f,-6.0f,"SPACE      :  Pause / Resume",GLUT_BITMAP_HELVETICA_12);
    renderText(-16.5f,-9.0f,"R          :  Restart",GLUT_BITMAP_HELVETICA_12);
    renderText(-16.5f,-12.0f,"ESC        :  Quit",GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.1f,1,0.45f); renderText(-14.5f,-24,"Press SPACE to Start!",GLUT_BITMAP_TIMES_ROMAN_24);
}

/* Game Over screen - layout drafted, polish IN PROGRESS */
void drawGameOverScreen(int fs)
{
    char buf[64];
    glColor3f(0.08f,0.02f,0.02f);
    glBegin(GL_QUADS); glVertex2f(-35,-40); glVertex2f(35,-40); glVertex2f(35,40); glVertex2f(-35,40); glEnd();
    drawPanel(-20,-3,40,20,0.22f,0.02f,0.02f,0.94f);
    glColor3f(1,0.18f,0.18f); renderText(-9.5f,13,"GAME  OVER!",GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(1,1,0); sprintf(buf,"Final Score: %06d",fs); renderText(-11.5f,8,buf,GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.9f,0.9f,0.9f); renderText(-13.5f,3,"Press R   to Restart",GLUT_BITMAP_HELVETICA_18);
    renderText(-12,-1.5f,"Press ESC to Quit",GLUT_BITMAP_HELVETICA_12);
    /* TODO: add animated score tally effect - PLANNED */
}

/* ================================================================
   MEMBER 5 - GAME LOGIC & COLLISION
   STATUS: ~70% COMPLETE

   COMPLETED FEATURES:
   - Implemented AABB (Axis-Aligned Bounding Box) collision detection
     for efficient and accurate player-enemy interaction.
   - Added invincibility timer to prevent multiple rapid collisions
     within a short time frame.
   - Developed particle-based explosion system with directional
     movement and basic physics (velocity and gravity).
   - Designed and implemented a game state machine controlling
     transitions between START, PLAY, PAUSE, and GAME OVER states.

   CURRENT IMPROVEMENTS:
   - Explosion animation refined with smoother life decay and
     improved color fading for better visual quality.
   - Restart logic enhanced to ensure enemies spawn in unique lanes
     with proper vertical spacing, preventing overlap issues.

   FUTURE ENHANCEMENTS (PLANNED):
   - Level-up visual feedback (e.g., screen flash or animation).
   - Advanced collision handling (Swept-AABB) for high-speed accuracy.

   ================================================================ */
int invincibleTimer = 0;

typedef struct { float x,y,vx,vy,life,r,g,b,sz,angle,rotSpd; } Particle;
Particle particles[22];
int explosionActive = 0;

void spawnExplosion(float ex,float ey)
{
    int i;
    explosionActive=1;
    for(i=0;i<22;i++){
        float a=2*PI*(float)rand()/(float)RAND_MAX;
        float sp=0.18f+(float)(rand()%40)/100.0f;
        particles[i].x=ex;   particles[i].y=ey;
        particles[i].vx=sp*(float)cos(a);
        particles[i].vy=sp*(float)sin(a);
        particles[i].life=1.0f;
        particles[i].r=1;
        particles[i].g=0.25f+(float)(rand()%55)/100.0f;
        particles[i].b=0;
        particles[i].sz=0.45f+(float)(rand()%30)/100.0f;
        particles[i].angle=(float)(rand()%360);
        particles[i].rotSpd=5+(float)(rand()%14);
    }
}

void drawExplosion(void)
{
    int i, any=0;
    if(!explosionActive) return;

    for(i=0;i<22;i++){
        float lf=particles[i].life;
        if(lf<=0) continue;

        any=1;

        particles[i].x    += particles[i].vx;
        particles[i].y    += particles[i].vy;
        particles[i].vy   -= 0.010f;        /* slightly softer gravity */


        particles[i].life -= 0.015f;

        if(particles[i].life < 0) particles[i].life = 0;

        particles[i].angle += particles[i].rotSpd;


        glColor3f(particles[i].r * lf,
                  particles[i].g * lf * 0.7f,
                  0);

        glPushMatrix();
        glTranslatef(particles[i].x,particles[i].y,0);
        glRotatef(particles[i].angle,0,0,1);
        glScalef(particles[i].sz * lf,particles[i].sz * lf,1);

        glBegin(GL_QUADS);
            glVertex2f(-0.5f,-0.5f);
            glVertex2f( 0.5f,-0.5f);
            glVertex2f( 0.5f, 0.5f);
            glVertex2f(-0.5f, 0.5f);
        glEnd();

        glPopMatrix();
    }

    if(!any) explosionActive=0;
}

/*
 *  AABB COLLISION DETECTION (EP3 - Analysis of object overlap)
 *  Two rectangles overlap if and only if they overlap on BOTH axes.
 *  |ax - bx| < (ahw + bhw)  AND  |ay - by| < (ahh + bhh)
 */
int aabbCollide(float ax,float ay,float ahw,float ahh,
                float bx,float by,float bhw,float bhh)
{
    float dx=ax-bx; if(dx<0)dx=-dx;
    float dy=ay-by; if(dy<0)dy=-dy;
    return (dx<ahw+bhw)&&(dy<ahh+bhh);
}

void checkCollisions(void)
{
    int i;
    drawExplosion();
    updatePlayerCar();
    if(invincibleTimer>0){ invincibleTimer--; return; }
    for(i=0;i<3;i++){
        if(!enemies[i].active) continue;
        if(aabbCollide(playerX,playerY,2.0f,3.2f,
                       enemies[i].x,enemies[i].y,1.85f,3.0f)){
            lives--;
            spawnExplosion(playerX,playerY);
            enemies[i].y=55.0f;    /* push enemy off screen */
            if(lives<=0){ lives=0; gameState=STATE_OVER; return; }
            invincibleTimer=100;   /* ~1.6 seconds at 60fps */
        }
    }
}

void initGame(void)
{
    int i;
    score=0;
    lives=3;
    level=1;
    invincibleTimer=0;
    explosionActive=0;

    for(i=0;i<22;i++) particles[i].life=0;

    initPlayerCar();


    int used[3] = {0,0,0};

    for(i=0;i<3;i++){
        int lane;
        do {
            lane = rand()%3;
        } while(used[lane]);

        used[lane] = 1;

        enemies[i].lane = lane;
        enemies[i].x    = laneX[lane];

        enemies[i].y    = 50.0f + i * 20.0f;   /* spaced vertically */
        enemies[i].speed= 0.18f + (float)(rand()%22)/100.0f;

        enemies[i].active = 1;
    }

    initBackground();
}

/* ================================================================
   MAIN - GLUT setup, display callback, timer, keyboard
   ================================================================ */

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if(gameState==STATE_START)
    {
        drawStartScreen();
    }
    else if(gameState==STATE_PLAY || gameState==STATE_PAUSE)
    {
        /* Draw scene layers back to front */
        drawSky();
        drawRoad();
        drawRoadMarkings();
        drawSideTrees();
        drawClouds();
        drawAllEnemyCars();
        drawPlayerCar(playerX,playerY);
        /* HUD drawn on top */
        drawScore(score);
        drawLives(lives);
        drawLevel(level);
        if(gameState==STATE_PAUSE) drawPauseOverlay();
    }
    else if(gameState==STATE_OVER)
    {
        drawGameOverScreen(score);
    }

    glutSwapBuffers();
}

void timerFunc(int v)
{
    if(gameState==STATE_PLAY){
        updateRoadMarkings();
        updateClouds();
        updateEnemyCars(level);
        checkCollisions();      /* also calls updatePlayerCar() */
        score++;
        if(score%300==0 && level<5) level++;
    }
    glutPostRedisplay();
    glutTimerFunc(16,timerFunc,0);   /* reschedule ~60fps */
}

void keyboard(unsigned char key,int x,int y)
{
    switch(key){
    case 'a': case 'A':
        if(gameState==STATE_PLAY) movePlayerLeft();
        break;
    case 'd': case 'D':
        if(gameState==STATE_PLAY) movePlayerRight();
        break;
    case ' ':
        if     (gameState==STATE_PLAY)       gameState=STATE_PAUSE;
        else if(gameState==STATE_PAUSE)      gameState=STATE_PLAY;
        else if(gameState==STATE_START){ gameState=STATE_PLAY; initGame(); }
        break;
    case 'r': case 'R':
        if(gameState==STATE_OVER){ gameState=STATE_PLAY; initGame(); }
        break;
    case 27: exit(0);   /* ESC */
    }
}

void specialKeys(int key,int x,int y)
{
    if(gameState!=STATE_PLAY) return;
    if(key==GLUT_KEY_LEFT)  movePlayerLeft();
    if(key==GLUT_KEY_RIGHT) movePlayerRight();
}

void reshape(int w,int h)
{
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(-35,35,-40,40,-1,1);
    glMatrixMode(GL_MODELVIEW);  glLoadIdentity();
}

int main(int argc,char** argv)
{
    srand((unsigned int)time(NULL));
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(700,800);
    glutInitWindowPosition(150,50);
    glutCreateWindow("Forza Horizon 2099  |  CSE422  |  DIU");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(-35,35,-40,40,-1,1);
    glMatrixMode(GL_MODELVIEW);  glLoadIdentity();

    initBackground();
    initPlayerCar();
    initEnemyCars();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16,timerFunc,0);
    glutMainLoop();
    return 0;
}
