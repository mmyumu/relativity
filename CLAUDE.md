# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Relativity2 is an Unreal Engine 5.7 spacetime simulation/visualization project. The Z axis represents time (in years — *ans* in French), creating a spacetime diagram where objects move through time as they travel along Z.

## Build & Development

This is a Windows-only UE5 project using Visual Studio. All builds go through UnrealBuildTool (UBT).

**Generate project files** (run when adding/removing .cpp/.h files):
```
"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="C:\MyProjects\Relativity2\Relativity2.uproject" -game -rocket -progress
```

**Build the editor module from command line:**
```
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" Relativity2Editor Win64 Development "C:\MyProjects\Relativity2\Relativity2.uproject" -waitmutex
```

**Typical workflow**: Open `Relativity2.sln` in Visual Studio, set startup project to `Relativity2Editor`, build with F7, launch with F5 to open the UE Editor.

There are no automated tests in this project — validation is done by running the editor and playing in the `RelativityMap` level.

## Architecture

### C++ Module (`Source/Relativity2/`)

The single runtime module depends only on `Core`, `CoreUObject`, `Engine`, and `InputCore`.

**`ATimeAxisActor`** (`TimeAxisActor.h/.cpp`) — the only C++ Actor. Renders a scrolling time axis with tick marks and year labels. Each frame it follows a designated `FollowActor`'s Z position, computing which tick indices are visible and repositioning `UStaticMeshComponent` tick marks and `UTextRenderComponent` labels dynamically. Key design: tick marks are created once in `BeginPlay()` via `CreateTicks()` and repositioned each tick in `UpdateAxis()` — avoid destroying/recreating them at runtime.

### Blueprint Layer (`Content/Blueprints/`)

- **BP_SimulationManager** — orchestrates the overall simulation state
- **BP_TimeTick** — represents a discrete event or tick in spacetime
- **BP_PlayerPoint** — the player's worldline point (used as the `FollowActor` for the time axis)
- **BP_Earth** — Earth worldline actor
- **BP_TimeAxis / BP_TimeAxis_CPP** — Blueprint and C++-backed variants of the time axis visualization
- **BP_GameMode_NoPawn** — game mode with no default pawn (simulation is observer-based, not player-controlled)

### Renderer Configuration

Ray tracing is enabled (DX12/SM6), Lumen GI and Substrate materials are active, and static lighting is disabled. All lighting must be dynamic.

## Key Conventions

- Z = time axis throughout the project (not the typical "up" axis). Keep this in mind when reading or writing actor positioning code.
- `TickSpacing` (default 100 UU) maps to 1 year in simulation space.
- `bClampAtZero` prevents the time axis from showing negative time values — maintain this guard when modifying `UpdateAxis()`.
- New C++ Actors must be added to `Relativity2.Build.cs` module dependencies only if they require additional UE modules beyond the existing four.
