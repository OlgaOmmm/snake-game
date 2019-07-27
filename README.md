# snake-game
mini-game Snake (C++, OpenGL, STL)
_____________________________________
Project includes files:

1) main.cpp

2) Shader.h (from learnopengl.com)

3) 2d.vs - vertex shader (from learnopengl.com)

4) 2d.fs - fragment shader (from learnopengl.com)

5) stb_image.h (download https://github.com/nothings/stb/blob/master/stb_image.h)

6) glad.c (download https://glad.dav1d.de/) - Go to the web service, make sure the language is set to C++ and in the API section, 
select an OpenGL version of at least 3.3 (which is what we'll be using for these tutorials; higher versions are fine as well). 
Also make sure the profile is set to Core and that the Generate a loader option is ticked. 
Ignore the extensions (for now) and click Generate to produce the resulting library files.
GLAD by now should have provided you a zip file containing two include folders, and a single glad.c file. 
Copy both include folders (glad and KHR) into your include(s) directory (or add an extra item pointing to these folders), 
and add the glad.c file to your project.

7) blueBall.jpg - my texture for game. You can use any other.

Also you need to include header files: glad.h, glfw3.h, glm.hpp, matrix_transform.hpp, type_ptr.hpp, Windows.h. Libraries: vector, ctime.
_____________________________________
Keyboard keys:

- direction of movement: keyboard arrows Up, Right, Down, Left.

- exit: Esc.
