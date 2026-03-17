# Forza Horizon 2099

> An interactive OpenGL car racing game — CSE422 Computer Graphics Lab Project

## About the Project

**Forza Horizon 2099** is a top-down, 2D, real-time car avoidance game built entirely with **C++ and OpenGL (FreeGLUT)**. The player controls a red sports car on a three-lane highway and must dodge three oncoming enemy cars for as long as possible. The score increases continuously while the game runs, and difficulty scales through five levels as enemy cars get progressively faster.

### Key Features
- Three-lane highway with real-time scrolling road markings
- Player car with smooth lane-change animation and rotation tilt
- Three enemy cars with level-based speed progression
- Explosion particle system on collision (translate + rotate + scale)
- Full game state machine: Start → Play → Pause → Game Over → Restart
- HUD overlay: score counter, lives (heart icons), level indicator
- Moving clouds and decorative side trees for environmental depth

---

## Graphics Algorithms

All three algorithms required by EP5 are implemented manually — no OpenGL built-in line functions are used.

### 1. DDA Line Algorithm
**Where:** `drawRoadMarkings()` — scrolling yellow lane divider dashes
**How it works:** Computes `dx` and `dy`, takes `steps = max(|dx|, |dy|)`, then increments x and y by `dx/steps` and `dy/steps` each step, plotting a `GL_POINT` at every position.
```cpp
// Each lane dash is drawn by one ddaLine() call
// dashOffset scrolls each frame, creating the driving illusion
ddaLine(lx[lane], y1, lx[lane], y2);
```

### 2. Bresenham Line Algorithm
**Where:** `drawEnemyCar()` — black outline on all 4 edges of every enemy car
**How it works:** Uses integer error term `err = dx - dy`. Each step: if `2*err > -dy`, advance in x; if `2*err < dx`, advance in y. No floating-point division.
```cpp
// Four edges of each enemy car drawn with Bresenham
bresenhamF(-hw,-hh,  hw,-hh);  // bottom
bresenhamF(-hw, hh,  hw, hh);  // top
bresenhamF(-hw,-hh, -hw, hh);  // left
bresenhamF( hw,-hh,  hw, hh);  // right
```

### 3. Midpoint Circle Algorithm
**Where:** `midpointCircleOutline()` — tyre outline on each of the 4 player car wheels
**How it works:** Starts at `(0, r)` with decision parameter `d = 1 - r`. Each iteration plots 8 symmetric points (one per octant). If `d < 0`: `d += 2*xi + 3`. Else: `d += 2*(xi-yi) + 5`, `yi--`.
```cpp
// 8 symmetric points plotted per iteration
glVertex2f(cx+rx,cy+ry); glVertex2f(cx-rx,cy+ry);
glVertex2f(cx+rx,cy-ry); glVertex2f(cx-rx,cy-ry);
glVertex2f(cx+ry,cy+rx); glVertex2f(cx-ry,cy+rx);
glVertex2f(cx+ry,cy-rx); glVertex2f(cx-ry,cy-rx);
```
---

## 2D Transformations

All transformations use `glPushMatrix()` / `glPopMatrix()` to isolate each object's transformation from all others.

| Transformation | Applied To | Code |
|----------------|-----------|------|
| **Translation** | Player car, enemy cars, clouds, HUD panels, explosion particles | `glTranslatef(x, y, 0)` |
| **Rotation** | Player car tilt on lane change, explosion particle spin | `glRotatef(angle, 0, 0, 1)` |
| **Scaling** | HUD panel sizing, explosion particle fade-out | `glScalef(w, h, 1)` |
| **Reflection** | Road symmetry (left/right sides mirror each other) | Symmetric coordinate values around x=0 |

### Highlight: Particle System (all 3 transforms simultaneously)
```cpp
glPushMatrix();
    glTranslatef(p.x, p.y, 0);             // fly to position
    glRotatef(p.angle, 0, 0, 1);           // spin
    glScalef(p.sz*p.life, p.sz*p.life, 1); // shrink as life fades
    // draw particle square
glPopMatrix();
```
---

## Animations

All animation is driven by `glutTimerFunc(16ms)` — approximately 60 fps.

| Animation | Technique | Speed |
|-----------|-----------|-------|
| Road lane dash scroll | `dashOffset += 0.40` per frame, wraps at 9.0 | Constant |
| Cloud movement | `cloud.x += speed` per frame, wraps at x=42 | 0.015 – 0.030 per frame |
| Enemy car movement | `enemy.y -= (speed + base)` per frame | Increases with level |
| Player lane change | Lerp: `playerX += (targetX - playerX) * 0.18` | Smooth exponential |
| Player tilt | `playerTilt *= 0.85` decay per frame | Fades out naturally |
| Explosion particles | Translate + rotate + scale per particle per frame | 22 particles, ~0.5s lifetime |

---

## Controls

| Key | Action |
|-----|--------|
| `←` Left Arrow / `A` | Move car to left lane |
| `→` Right Arrow / `D` | Move car to right lane |
| `SPACE` | Start game / Pause / Resume |
| `R` | Restart after Game Over |
| `ESC` | Quit |

---

## Build & Run

### Code::Blocks
1. Open Code::Blocks → **File → New → Empty Project**
2. Right-click project → **Add files** → select `main.cpp`
3. Go to **Project → Build Options → Linker Settings**
4. In **"Other linker options"** paste:
   ```
   -framework OpenGL -framework GLUT
   ```
5. Press **F9** to build and run

### If you see a "GLUT deprecated" warning
Add this to **Compiler flags**:
```
-Wno-deprecated-declarations
```
---

## Project Structure

```
forza-horizon-2099/
│
├── main.cpp          ← entire project in one file
├── README.md         ← this file
└── .gitignore        ← ignores compiled binaries and IDE files
```
---

*CSE422 Computer Graphics Lab — Spring 2026 — Daffodil International University*
