# UE5-Spider-Procedural-Locomotion

A procedural spider locomotion system built in Unreal Engine 5 using C++, Blueprints, AI Behavior Trees, and Control Rig.

The system allows a spider character to traverse floors, walls, ceilings, and uneven terrain while maintaining its orientation to the current surface. The locomotion system can be driven by either AI behavior or direct player input.


**Demo Overview**

This project contains two demonstration levels built around the same underlying locomotion system.


**AI-Controlled Spider**

AI-controlled spiders detect the player and pursue them through the environment, transitioning between floors, walls, ceilings, and uneven terrain. The AI will also drop from the ceiling if the player is in range.

The purpose of this demo is to show how the locomotion system integrates with AI-driven movement and navigation.



**Player-Controlled Spider**

The player directly controls the spider and can navigate across the same types of climbable surfaces.

This demonstrates that the locomotion system is not dependent on the AI implementation and can respond to different sources of movement input.



**Core Technologies**
Unreal Engine 5
C++
Blueprints
AI Behavior Trees
Control Rig


**How the System Works**

The core climbing system is handled by the Blueprint Component \[SpiderPawnMovement], keeping the system modular and requiring only a Normal trace input from the blueprint side.


At a high level, the system:

Samples and averages surface normals around the pawn to determine the current surface direction.
Inverts that averaged normal to generate the local downward force.
Determines the pawn’s desired orientation relative to the detected surface.
Projects movement along that orientation so the pawn can move across the surface.
Handles transitions between floors, walls, ceilings, and uneven terrain.
Accepts movement input from either player control or AI.
Passes locomotion data to the procedural animation system responsible for positioning the spider’s legs.



**Running the Demo**
Open ProceduralSpiderDemo.uproject in Unreal Engine 5.7.
Open \[Starting Level].
Press Play.
Choose either the AI Controlled or Player Controlled demo.



**The current demo is intended to demonstrate:**

Floor, wall, and ceiling traversal
Surface transitions
Uneven-terrain traversal
AI-driven movement
Player-driven movement
Integration between C++, Blueprints, AI, and procedural animation



**Technical Breakdown**

A more detailed breakdown of the system, including video demonstrations and implementation explanations, is available on my portfolio:

Portfolio Breakdown: https://www.christianwheelerxr.com/proceduralspiderlocomotionsystem


**About**
This project was developed as part of my ongoing work in gameplay programming and Unreal Engine, with a focus on movement systems, AI behavior, and procedural locomotion.


Christian Wheeler
Gameplay Programmer

