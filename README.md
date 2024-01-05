# Raindrops In Motion COMP3016 70%

Raindrops in motion is a rain simulation prototype made in OpenGL C++, It uses mouse movement, camera movement, creation of plane and cube and the use of a particle system to allow for "rain" to occur.

# How To Run The Prototype

**To run the prototype:**
1. Open the "BuildEXE" Folder
2. Move the "OpenGL" Folder into C:\Users\Public
3. Launch COMP3016.exe within the "BuildEXE" Folder

**Controls**
W - Forward movement
A - Left movement
S - Backwards movement
D - Right movement

Mouse - Look around


## How My Prototype Works

**Main Function (main):**

-   Initializes GLFW.
-   Creates a GLFW window for rendering.
-   Initializes GLEW.
-   Error handling if Library fails to Initiate.
-   Loads a texture from an image file.
-   Compiles and links shaders.
-   Sets up vertex data and buffers for a plane and a quad.
-   Initializes particle positions and velocities.
-   Enters the main rendering loop.

**Shader Loading:**
-   Reads shader source code from files using `readShaderFile` function.
-   Compiles vertex and fragment shaders.
-   Links shaders into a shader program.

**Rendering Loop:**
-   Calculates frame timing using `glfwGetTime`.
-   Processes user input using `processInput`.
-   Clears the screen.
-   Updates particle positions using `updateParticles`.
-   Renders particles, plane, and quad using appropriate shader programs and VAOs.

**Particle Initialization and Update Functions**:
-   `initializeParticles`: Initializes an array of particle positions and velocities.
-   `updateParticles`: Updates particle positions based on their velocities.

**Input Handling:**
- `processInput`: Handles user input for camera movement.

## How I Approached The Task

I believe that Rain Simulation has some use cases, this includes game development and virtual environments and can be further used in real world cases.

When approaching this prototype I took it in steps to get to my final product.
1. I created a GLFW Window and got a simple green plane to appear
2. I created mouse and keyboard movement to allow the user to simulate walking across the plane and looking around
3.  I added a texture to my plane to make the ground feel like grass like the user is actually walking along grass.
4.  Added particle system to imitate rain and added rain colour
5.  Created a cube to imitate a bush and applied texture to it

I believe my prototype is efficient with the code, load time is instant and gameplay is smooth. Using a particle system instead of individually loading rain models and generating them in large quantities allowed me to have quick load times. I have implemented logic in the particles to reset their positions when hit the ground and allow for different particle positions when spawning and falling making it seem more like rain.

## Links

GITHUB Repo: https://github.com/CallumH03/COMP3016-70
Video Report: 

## References
**Sites**
Learnopengl - https://learnopengl.com/
Stackedit - https://stackedit.io/app

**Libraries**
GLFW - https://www.glfw.org/
GLEW - https://glew.sourceforge.net/
GLM - https://github.com/g-truc/glm
STB_IMAGE.H - https://github.com/nothings/stb/blob/master/stb_image.h