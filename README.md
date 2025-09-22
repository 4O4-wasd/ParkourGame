# ParkourGame (aka J.A.P.G)

### This is a learning project so expect bugs and jankiness.

An FPS parkour game prototype that is made in Unreal Engine 5.
This project was initially created as a way for me to learn Unreal Engine and C++ but I guess it evolved into a derpy and fun game (in my opinion).

[![Play on Itch.io](https://img.shields.io/badge/Itch.io-Play%20J.A.P.G%20Now-FA5C5C?logo=itch.io&logoColor=white)](https://4o4-wasd.itch.io/parkour-game)

---

## Features

### Movement

-   Walking – default movement
-   Crouching – lower stance
-   Sprinting – moving faster
-   Sliding – momentum-based move triggered while sprinting + crouching
-   Vaulting – climbing over obstacles
-   WallRunning – running along walls
-   Dashing – quick burst in a direction, air or ground
-   Grappling Gun (Swinger) – swing across gaps and obstacles

### Combat

-   **Modular Weapon System** – advanced setup that supports:
    -   Configurable damage, range, fire rate, magazine size
    -   Random spread and multiple fire modes (e.g., single shot)
    -   PushBack recoil, rotation recoil, and camera recoil
    -   Recoil snappiness and return speed for smoother gunplay
    -   Hooks for sounds, animations, muzzle flashes, shell ejections, bullet traces, and impact effects
    -   Reloading logic with ammo tracking (though ammos are unlimited)
-   **Health System** – integrated with the weapon system so both the player and AI can take damage, die, or kill each other

### AI

-   J.A.A.I.E – a simple but overpowered AI that shoots at the player
-   Boom Boy AI – runs at you and explodes if it gets close

### UI

-   Main Menu – entry point of the game
-   Settings Menu – adjust basic game options
-   Map Menu – selecting levels

### Levels

-   Tutorial Level – learn the basics of movement
-   3 Playable Maps – test your parkour

---

## Stack

-   Engine – Unreal Engine 5.5
-   Languages – C++ (mainly), Blueprint

---

## Project Size

This repo has a lot of bloat as this is a learning project, I didn’t optimize the files or clean up unused assets. If you clone it then expect a large download and some messy code/organization.

---

## Inspiration

-   [Mirror’s Edge](https://en.wikipedia.org/wiki/Mirror%27s_Edge) – for fluid parkour movement
-   [Karlson](https://store.steampowered.com/app/1228610/KARLSON/) – for momentum-based movement and goofy AI ideas
