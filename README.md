# Forza Horizon 2099
### CSE422 — Computer Graphics Lab | Spring 2025
**Daffodil International University | Department of CSE**

---

## Project Overview
An interactive OpenGL car racing game built with C++ and FreeGLUT.
The player controls a red car on a 3-lane highway, avoiding oncoming
traffic to survive as long as possible.

## Current Progress: ~60%
| Module | Member | Status |
|--------|--------|--------|
| Background (Sky, Road, DDA Lane Markings, Clouds, Trees) | Member 1 | ✅ Complete |
| Player Car (Body, Midpoint Circle Wheels, Movement) | Member 2 | ✅ Complete |
| Enemy Cars (Bresenham Outlines, Auto-movement) | Member 3 | ✅ Complete |
| HUD & UI Screens (Score, Lives, Level, Screens) | Member 4 | ⚙️ ~70% |
| Collision Detection & Game Logic | Member 5 | ⚙️ ~40% |

## Algorithms Implemented (EP5)
- **DDA Line Algorithm** — scrolling yellow lane divider dashes
- **Bresenham Line Algorithm** — enemy car body outlines
- **Midpoint Circle Algorithm** — player car wheel tyre outlines

## 2D Transformations Used (EP1 / EP3)
- `glTranslatef` — all moving objects (cars, clouds, HUD panels, particles)
- `glRotatef` — player car tilt on lane change, explosion particle spin
- `glScalef` — HUD panel sizing, explosion particle fade

## Controls
| Key | Action |
|-----|--------|
| Left Arrow / A | Move car left |
| Right Arrow / D | Move car right |
| SPACE | Start / Pause / Resume |
| R | Restart (after Game Over) |
| ESC | Quit |

## Build Instructions

**Mac (Terminal):**
```bash
g++ main.cpp -framework OpenGL -framework GLUT -Wno-deprecated-declarations -o game && ./game
```

**Mac (Code::Blocks):**
1. Create a new empty project
2. Add `main.cpp`
3. Go to Project → Build Options → Linker Settings
4. Add to "Other linker options": `-framework OpenGL -framework GLUT`
5. Press F9

## Team Members
| Role | Name | ID |
|------|------|----|
| Member 1 — Background | [Name] | [ID] |
| Member 2 — Player Car | [Name] | [ID] |
| Member 3 — Enemy Cars | [Name] | [ID] |
| Member 4 — HUD & UI | [Name] | [ID] |
| Member 5 — Collision & Logic | [Name] | [ID] |

**Instructor:** Tapasy Rabeya | Semester: Spring 2025
