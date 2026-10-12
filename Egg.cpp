#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <glm/glm/gtx/quaternion.hpp>
#include <glm/glm/gtc/quaternion.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <dualnum.h>
#include "text_rendering.h"
#include "IBL.h"
#include "Textures.h"
#include "Primitives.h"
#include "RenderTargets.h"
#include "Renderer.h"
#include "GameEnums.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
glm::mat4 GetCurrentArcballRotation();
glm::quat ComputeArcballRotationQuaternion(float mousexcurrent, float mouseycurrent, float mousexstart, float mouseystart);
void raycast(float mouse_x, float mouse_y, glm::mat4 view_matrix, glm::mat4 projection_matrix, glm::mat4 model_matrix, glm::vec3& origin, glm::vec3& direction);

//enums
VisualMode vmode = NORMAL;
InteractionMode imode = ROTATE;
GameState gmode = ACTIVE;

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
int USE_NORMALS = 0;

//hdr
bool USE_HDR = false; //best to leave this off, with just one light in the scene the tonemapping makes it look worse. also doesnt work with pbr mode currently
bool bloom = false;
float exposure = 1.0f; //for tone mapping

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

//sphere
glm::mat4 sphereModel = glm::mat4(1.0f);
glm::mat4 sphereView = camera.GetViewMatrix();
glm::mat4 sphereProjection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

//arcball stuff
//glm::quat last_arcball_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
glm::quat last_arcball_rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //set an intial position
glm::quat current_arcball_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
float pxstart, pystart, pxcurrent, pycurrent;

float sunAngle = 0.0f; //sun rotation angle

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float endTimer = 0.0f;
bool dimensionsChanged = false; //scuffed toggle to update renderer dimensions inside render loop 

// lighting info
// -------------
glm::vec3 lightPos(1.0f, 3.0f, 2.0f);


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_SAMPLES, 4); // request 4x MSAA

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "egg", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    //msaa
    //glEnable(GL_MULTISAMPLE);


    // build and compile shaders
    //these are needed to construct the ibl maps so they exist outside the renderer class
    Shader equirectangularToCubemapShader("shaders/generate_cubemap.vs", "shaders/hdr_to_cubemap.fs");
    Shader irradianceShader("shaders/generate_cubemap.vs", "shaders/irradiance_convolution.fs");    
    Shader prefilterShader("shaders/generate_cubemap.vs", "shaders/prefilter.fs");
    Shader brdfShader("shaders/brdf.vs", "shaders/brdf.fs");

    //text shader
    //handled outside renderer class because its simple
    Shader textShader("shaders/text.vs", "shaders/text.fs");
    glm::mat4 textprojection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(textprojection));
    unsigned int textVAO, textVBO;
    std::map<GLchar, Character> Characters;
    int textErrors;
    textErrors = InitializeFreetype(textVAO, textVBO, Characters);

    //build sphere
    dualnum::Sphere dSphere(250, window);

    //model, also handled outside renderer class because im lazy and did this last
    Shader sunShader("shaders/sun.vs", "shaders/sun.fs");
    Model sun("sun/UnstableStar.fbx"); //model from SebastianSosnowski : https://sketchfab.com/3d-models/sun-9ef1c68fbb944147bcfcc891d3912645

    // set up vertex data/buffers and configure vertex attributes
    Primitives prims = BuildPrimitives();
    //shadowmap, post process, hdr fbos
    const unsigned int SHADOW_WIDTH = 1000, SHADOW_HEIGHT = 1000;
    RenderTargets renderTargets = BuildRenderTargets(SCR_WIDTH, SCR_HEIGHT, SHADOW_WIDTH, SHADOW_HEIGHT);

    //IBL stuff/PBR setup================================
     IBLMaps ibl = BuildIBL("resources/HDR_white_local_star_and_nebulae.hdr",
    { equirectangularToCubemapShader, irradianceShader, prefilterShader, brdfShader },
         prims.skyboxVAO, prims.quadVAO);
     //"resources/newport_loft.hdr"
        if (!ibl.valid())
            std::cout << "PBR mode will render without environment lighting" << std::endl;

    //================================
    // load textures
    // -------------
    unsigned int cubeTexture = loadTexture("resources/container.jpg"); 
    unsigned int sphereTexture = loadTexture("metal/Metal046B_2K-JPG_Color.jpg");//diffuse texture
    unsigned int normalTexture = loadTexture("metal/Metal046B_2K-JPG_NormalGL.jpg"); //sphere normal map
    unsigned int depthTexture = loadTexture("metal/Metal046B_2K-JPG_Displacement.jpg"); //sphere depth map
    unsigned int floorTexture = loadTexture("resources/whitesquare.jpg");
    unsigned int roughnessMap = loadTexture("metal/Metal046B_2K-JPG_Roughness.jpg"); 
    unsigned int metalnessMap = loadTexture("metal/Metal046B_2K-JPG_Metalness.jpg"); 
    unsigned int sunTex = loadTexture("sun/suncyl1.jpg", "sun");
    vector<std::string> faces =
    {
        "skybox/px.png",
        "skybox/nx.png",
        "skybox/py.png",
        "skybox/ny.png",
        "skybox/pz.png",
        "skybox/nz.png"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    SceneTextures sceneTextures{ sphereTexture, normalTexture, depthTexture, cubeTexture, floorTexture, cubemapTexture, roughnessMap, metalnessMap };
    Renderer renderer(prims, renderTargets, ibl, dSphere, sceneTextures,
        SCR_WIDTH, SCR_HEIGHT, SHADOW_WIDTH, SHADOW_HEIGHT);


    // render loop; note: game logic largely lives in the callbacks because... it was simple i guess
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);
        
        //update dimensions if applicable
        if (dimensionsChanged == true) {
            sphereProjection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            renderer.UpdateDimensions(SCR_WIDTH, SCR_HEIGHT);
            ResizeRenderTargets(renderTargets, SCR_WIDTH, SCR_HEIGHT);
            glm::mat4 textprojection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
            textShader.use();
            glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(textprojection));
            dimensionsChanged = false;
        }

        // render
        //clear framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);

        if(USE_HDR){
        glBindFramebuffer(GL_FRAMEBUFFER, renderTargets.hdr.FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        //set uniforms
        sphereModel = GetCurrentArcballRotation();
        float parallaxScaleFactor = 0.035f; //can probably hardcode this into renderer.cpp but leaving it out here for now in case i want to play with it via a slider or keypress

        if (gmode == INACTIVE) { endTimer += deltaTime; }
        renderer.BeginFrame(camera, view, projection, sphereModel, sphereView, sphereProjection, lightPos, parallaxScaleFactor);
        renderer.RenderFrame(vmode, gmode, endTimer, USE_HDR, USE_NORMALS, bloom, exposure);
        
        if (gmode == INACTIVE){
        sunShader.use();
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("view", view); 
        sunShader.setInt("texture_diffuse1", 0);        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTex);
        sunAngle += deltaTime;
        sunAngle = (sunAngle > 360.0f ? sunAngle - 360.0f : sunAngle);
        sunShader.use();
        model = glm::mat4(1.0f);
        //TRS since glm postmultiplies
        model = glm::rotate(model, glm::radians(sunAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));        
        model = glm::scale(model, glm::vec3(0.09f, 0.09f, 0.09f)); //sun model spans +/- 10.0f in any direction
        sunShader.setMat4("model", model);
        sun.Draw(sunShader);
        }

        //draw text
        if (gmode==ACTIVE){
        RenderText(textShader, "1-4 for visual modes", (float)SCR_WIDTH - 240.0f, (float)SCR_HEIGHT - 30.0f, 0.35f, glm::vec3(0.3, 0.7f, 0.9f), textVAO, textVBO, Characters);
        RenderText(textShader, "r: rotate", (float)SCR_WIDTH - 240.0f, (float)SCR_HEIGHT - 50.0f, 0.35f, glm::vec3(0.3, 0.7f, 0.9f), textVAO, textVBO, Characters);
        RenderText(textShader, "f:interact", (float)SCR_WIDTH - 240.0f, (float)SCR_HEIGHT - 70.0f, 0.35f, glm::vec3(0.3, 0.7f, 0.9f), textVAO, textVBO, Characters);
        }
        else if (gmode == INACTIVE && endTimer > 5.0f) { RenderText(textShader, "ESC to exit", (float)SCR_WIDTH/2.0f - 20.0f, (float)SCR_HEIGHT/2.0f, 0.55f, glm::vec3(1.0f, 0.0f, 0.5f), textVAO, textVBO, Characters); }
        
        //swap buffers and poll IO events 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //deallocate resources     
    DestroyPrimitives(prims);
    DestroyRenderTargets(renderTargets);
    DestroyIBL(ibl);
    glfwTerminate();
    return 0;
}

//==============arcball/ vertex selection stuff=======
glm::mat4 GetCurrentArcballRotation()
{
    return glm::toMat4(current_arcball_rotation * last_arcball_rotation);

    ////alternatively do it manually
    //glm::quat q = current_arcball_rotation * last_arcball_rotation;
    //glm::vec4 col0(1.0f - 2 * (q.y * q.y + q.z * q.z), 2 * (q.x * q.y + q.z * q.w), 2 * (q.x * q.z - q.y * q.w), 0.0f);
    //glm::vec4 col1(2 * (q.x * q.y - q.z * q.w), 1.0f - 2 * (q.x * q.x + q.z * q.z), 2 * (q.y * q.z + q.x * q.w), 0.0f);
    //glm::vec4 col2(2 * (q.x * q.z + q.y * q.w), 2 * (q.y * q.z - q.x * q.w), 1.0f - 2 * (q.x * q.x + q.y * q.y), 0.0f);
    //glm::vec4 col3(0.0f, 0.0f, 0.0f, 1.0f);
    //return glm::mat4(col0, col1, col2, col3);
}
glm::quat ComputeArcballRotationQuaternion(float mousexcurrent, float mouseycurrent, float mousexstart, float mouseystart)
{
    float ndc_x_start = mousexstart / SCR_WIDTH * 2 - 1; //convert from screen coords to ndc
    float ndc_y_start = -1 * (mouseystart / SCR_HEIGHT * 2 - 1); //negate y values so rotation isnt inverted
    float ndc_x_current = mousexcurrent / SCR_WIDTH * 2 - 1;
    float ndc_y_current = -1 * (mouseycurrent / SCR_HEIGHT * 2 - 1);
    //calc z component
    float ndc_z_start = (ndc_x_start * ndc_x_start + ndc_y_start * ndc_y_start > 1 ? 0 : sqrt(1 - ndc_x_start * ndc_x_start - ndc_y_start * ndc_y_start));
    float ndc_z_current = (ndc_x_current * ndc_x_current + ndc_y_current * ndc_y_current > 1 ? 0 : sqrt(1 - ndc_x_current * ndc_x_current - ndc_y_current * ndc_y_current));
    glm::vec3 pstart = glm::vec3(ndc_x_start, ndc_y_start, ndc_z_start);
    glm::vec3 pcurrent = glm::vec3(ndc_x_current, ndc_y_current, ndc_z_current);
    if (pstart == pcurrent) { return glm::quat(1.0f, 0.0f, 0.0f, 0.0f); } // fixes a bug when mouse click and mouse movement occur on same frame
    float theta = std::acos(std::min(1.0f, (glm::dot(pstart, pcurrent) / (glm::length(pstart) * glm::length(pcurrent))))); //rotation angle
    glm::vec3 u = glm::normalize(glm::cross(pstart, pcurrent)); //unit rotation axis
    glm::quat q = glm::quat(cos(theta / 2), u.x * sin(theta / 2), u.y * sin(theta / 2), u.z * sin(theta / 2)); //construct rotation quaternion from axis and angle
    return q;
}
void raycast(float mouse_x, float mouse_y, glm::mat4 view_matrix, glm::mat4 projection_matrix, glm::mat4 model_matrix, glm::vec3& origin, glm::vec3& direction)
{
    float ndc_x = mouse_x / SCR_WIDTH * 2 - 1; //convert from screen coords to ndc
    float ndc_y = -1 * (mouse_y / SCR_HEIGHT * 2 - 1); //need to flip y

    glm::vec4 ray_clip = glm::vec4(ndc_x, ndc_y, -1.0f, 1.0f); //dummy z/w value; dont need to reverse perspective division for direction vector
    glm::vec4 ray_camera = glm::inverse(projection_matrix) * ray_clip;
    ray_camera = glm::vec4(ray_camera.x, ray_camera.y, -1.0f, 0.0f); //camera points in -z dir
    glm::vec4 ray_world = glm::inverse(view_matrix) * ray_camera; //stop here with xyz and normalize if doing in world space
    glm::vec3 ray_model = glm::normalize(glm::vec3(glm::inverse(model_matrix) * ray_world)); //ray direction in modelspace
    glm::vec3 ray_origin_modelspace = glm::vec3(glm::inverse(model_matrix) * glm::vec4(camera.Position, 0.0f)); //origin of ray(camera origin) in model space
    origin = ray_origin_modelspace;
    direction = ray_model;
}

//========callbacks=============
//process all input, keys pressed
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    ////translational movement not needed in actual gameplay, but useful for debugging, hence commented out
    //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //    camera.ProcessKeyboard(FORWARD, deltaTime);
    //if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //    camera.ProcessKeyboard(BACKWARD, deltaTime);
    //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //    camera.ProcessKeyboard(LEFT, deltaTime);
    //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //    camera.ProcessKeyboard(RIGHT, deltaTime);
    //if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    //    camera.ProcessKeyboard(UP, deltaTime);
    //if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    //    camera.ProcessKeyboard(DOWN, deltaTime);
    
    //modes
    if (gmode==ACTIVE){
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        imode = ROTATE;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        imode = CLICK;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        vmode = NORMAL;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        vmode = SOBEL;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        vmode = GLASSES;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        vmode = PBR;
    }
}
//whenever the window size changed this is called
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);    
    dimensionsChanged = true;
}
// whenever mouse moves this is called
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    //camera.ProcessMouseMovement(xoffset, yoffset);

    //arcball stuff
    if (imode == ROTATE)
    {
        int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (state == GLFW_PRESS)
        {
            pxcurrent = xpos;
            pycurrent = ypos;
            current_arcball_rotation = ComputeArcballRotationQuaternion(pxcurrent, pycurrent, pxstart, pystart);
        }
    }
}
//called when mouse clicks
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double mouseX, mouseY;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &mouseX, &mouseY);
        pxstart = static_cast<float>(mouseX);
        pystart = static_cast<float>(mouseY);

    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        if (imode == ROTATE && gmode == ACTIVE) {
            last_arcball_rotation = current_arcball_rotation * last_arcball_rotation;
            current_arcball_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }
        else if (imode == CLICK && gmode == ACTIVE) {
            glm::vec3 ray_origin, ray_direction;
            raycast(pxstart, pystart, sphereView, sphereProjection, sphereModel, ray_origin, ray_direction);
            dualnum::Sphere* pdSphere = reinterpret_cast<dualnum::Sphere*>(glfwGetWindowUserPointer(window)); //need to store user pointer because dSphere needs glfw context from within main() and cant be made global easily
            if (pdSphere) {
                int checkindex = pdSphere->RaycastNearestVertex(ray_origin, ray_direction);
                //std::cout << checkindex << std::endl;
                if (checkindex > 0) {
                    pdSphere->DeformFromVertex(checkindex);
                    bool testerbool = pdSphere->DFSDriver();
                    //std::cout << "loop" << testerbool << std::endl;
                    if (testerbool == true) { 
                        //pdSphere->EggSeparator(pdSphere->cpath); 
                        gmode = INACTIVE;
                    }
                }
            }
        }
    }
}
// whenever the mouse scroll wheel scrolls, this callback is called; currently doesnt serve much of a purpose
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

