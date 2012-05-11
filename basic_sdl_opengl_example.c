
#include "common.c"

SDL_Window * mainwindow;   /* Our window handle */
SDL_GLContext maincontext; /* Our opengl context handle */
Uint32 then, now, frames;  /* Used for FPS */

#include "SDL_config.h"

/* cleanup before quiting */
static int
cleanup(int rc)
{
    /* Print out some timing information */
    now = SDL_GetTicks();
    if (now > then) {
        LOGE("%2.2f frames per second\n",
               ((double) frames * 1000) / (now - then));
    }
    if(maincontext)
        SDL_GL_DeleteContext(maincontext);
    if(mainwindow)
        SDL_DestroyWindow(mainwindow);
    SDL_Quit();
    exit(0);
}


int main(int argc, char** argv)
{
    int windowWidth = 512;
    int windowHeight = 1024;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { /* Initialize SDL's Video subsystem */
        LOG("Unable to initialize SDL");
        return cleanup(0);
    }

    /* Create our window centered at 512x512 resolution */
    mainwindow = SDL_CreateWindow("Simple rotating texture", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!mainwindow) {/* Die if creation failed */
        LOG("Unable to create window");
        return cleanup(0);
    }

    checkSDLError(__LINE__);

    /* Create our opengl context and attach it to our window */
    maincontext = SDL_GL_CreateContext(mainwindow);
    checkSDLError(__LINE__);

    if (!maincontext) {
        return cleanup(0);
    }

    // create the shaders
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

    vertexShader = loadShader(GL_VERTEX_SHADER, "vertex-shader-1.vert");
    checkGlError(__LINE__);
    fragmentShader = loadShader(GL_FRAGMENT_SHADER, "texture-shader-1.frag");
    checkGlError(__LINE__);
    programObject = glCreateProgram();
    if(programObject == 0) {
        LOGE("Unable to initialize the shader programm");
        return cleanup(0);
    }

    if(programObject == 0)
        return 0;

    checkGlError(__LINE__);
    glAttachShader(programObject, vertexShader);
    checkGlError(__LINE__);
    glAttachShader(programObject, fragmentShader);
    checkGlError(__LINE__);

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
    if(!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 1)
        {
            char* infoLog = malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            LOGE("Error linking program:\n%s\n", infoLog);
            free(infoLog);
        }
        glDeleteProgram(programObject);
        return 0;
    }

    checkGlError(__LINE__);
    // You need to 'use' the program before you can get it's uniforms.
    glUseProgram(programObject);
    checkGlError(__LINE__);

    GLuint gvPositionHandle = glGetAttribLocation(programObject, "a_position");
    // gvNormalHandle=glGetAttribLocation(gProgram,"a_normal");
    GLuint gvTexCoordHandle = glGetAttribLocation(programObject, "a_texCoord");
    GLuint gvSamplerHandle = glGetUniformLocation(programObject, "s_texture");

    //LOGE("a_position %d\n", gvPositionHandle);
    //LOGE("a_texCoord %d\n", gvTexCoordHandle);
    //LOGE("s_texture %d\n", gvSamplerHandle);

    // load texture
    GLuint textureid;
    struct textureInfos texture;
    texture.filename = "SDL_logo.bmp";
    loadTexture(&texture);
    glBindTexture( GL_TEXTURE_2D, texture.texture );

    checkGlError(__LINE__);
    checkSDLError(__LINE__);

    // setup the viewport
    glViewport( 0, 0, windowWidth, windowHeight );
    glClear( GL_COLOR_BUFFER_BIT );
    // Swap our back buffer to the front
    SDL_GL_SwapWindow(mainwindow);
    glClear(GL_COLOR_BUFFER_BIT);
    // enable blending
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_Event event;
    float theta = 0;
    int done = 0;
    then = SDL_GetTicks();
    float imagew_2 = texture.width / 2;
    float imageh_2 = texture.height / 2;

    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    checkGlError(__LINE__);
    GLfloat vVertices[] = {
        -0.75f, 0.75f, 0.0f, // Position 0
        //0.0f,1.0f,0.0f,
        0.0f, 0.0f, // TexCoord 0
        -.75f, -0.75f, 0.0f, // Position 1
        //0.0f,1.0f,0.0f,
        0.0f, 1.0f, // TexCoord 1
        .75f, -0.75f, 0.0f, // Position 2
        //0.0f,1.0f,0.0f,
        1.0f, 1.0f, // TexCoord 2
        .75f, 0.75f, 0.0f, // Position 3
        // 0.0f,1.0f,0.0f,
        1.0f, 0.0f // TexCoord 3
    };
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
    GLsizei stride = 5 * sizeof(GLfloat); // 3 for position, 2 for texture

    GLuint wtex;
    wtex = createWhiteTexture(wtex);

    while (!done) {
        ++frames;
        theta = theta + 0.1;
        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_WINDOWEVENT_CLOSE || event.type == SDL_QUIT) {
                done = 1;
            }
        }

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        checkGlError(__LINE__);

        // Load the vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                        vVertices);
                        checkGlError(__LINE__);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                    vVertices+3);
        // Load the texture coordinate
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                        vVertices+6);

        checkGlError(__LINE__);

        glEnableVertexAttribArray(gvPositionHandle);
        //glEnableVertexAttribArray(gvNormalHandle);
        glEnableVertexAttribArray(gvTexCoordHandle);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, texture.texture);
        glBindTexture(GL_TEXTURE_2D, wtex);


        // Set the sampler texture unit to 0
        glUniform1i(gvSamplerHandle, 0);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        checkGlError(__LINE__);

        SDL_GL_SwapWindow(mainwindow);
        SDL_Delay(5000);
        done = 1;
    }

    glDeleteTextures( 1, &textureid );
    return cleanup(0);
}
