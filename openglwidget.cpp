#include "openglwidget.h"

#include <QOpenGLShader>

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_vbo(QOpenGLBuffer::VertexBuffer)
    , m_shaderProgram(nullptr)
    , m_aspectRatio(1.0f)
    , m_rotationX(0.0f)
    , m_rotationY(0.0f)
    , m_mousePressed(false)
    , m_zoom(5.0f)
{
}

OpenGLWidget::~OpenGLWidget()
{
    makeCurrent();
    delete m_shaderProgram;
    m_vbo.destroy();
    m_vao.destroy();
    doneCurrent();
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.1f,0.1f,0.1f,1.0f);
    glEnable(GL_DEPTH_TEST);
    m_shaderProgram = new QOpenGLShaderProgram(this);
    const char *vertexShaderSource =
        R"(
            #version 330 core
            layout(location = 0) in vec3 position;
            layout(location = 1) in vec3 normal;
            layout(location = 2) in vec3 color;
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            out vec3 fragPos;
            out vec3 fragNormal;
            out vec3 fragColor;
            void main()
            {
                // STEP 1: TRANSFORM VERTEX POSITION TO WORLD SPACE
                // 'position' is the original position stored in our VBO.
                // Example: position = (-1, -1, 1)
                // The model matrix applies the object's transformation.
                // Result:
                //     local/object space
                //              ↓
                //          model matrix
                //              ↓
                //          world space
                //
                // We use w = 1.0 because this is a POSITION.
                vec4 worldPosition = model * vec4(position, 1.0);

                // STEP 2: SAVE WORLD POSITION
                // The fragment shader needs to know the position of the surface point in world space.
                // Why?
                // Because lighting calculations need to know:
                //     Where is the surface?
                //     Where is the light?
                // We remove the 'w' component because lighting works with a 3D position (x, y, z).
                fragPos = worldPosition.xyz;

                // STEP 3: TRANSFORM THE NORMAL
                // A normal tells us which direction a surface is facing.
                // Example:
                // Front face normal: (0, 0, 1)
                // If the cube rotates, its normal must rotate too.
                // We CANNOT always transform a normal using simply: mat3(model) * normal because scaling/transformations can distort the normal.
                // The correct normal transformation is: transpose(inverse(model))
                // This is called the NORMAL MATRIX.
                // mat3(...) removes the translation part because normals represent directions, not positions.
                fragNormal = mat3(transpose(inverse(model))) * normal;

                // Send vertex color to fragment shader
                fragColor = color;

                // Convert to screen position
                gl_Position = projection * view * worldPosition;
            }
        )";
    const char *fragmentShaderSource =
        R"(
            #version 330 core
            in vec3 fragPos;
            in vec3 fragNormal;
            in vec3 fragColor;
            uniform vec3 lightPos;
            uniform vec3 lightColor;
            uniform vec3 viewPos;
            out vec4 finalColor;
            void main()
            {
                // PART 1: AMBIENT LIGHTING
                // Ambient light represents a small amount of general light that exists everywhere in the scene.
                // Without ambient light, surfaces that do not directly face the light could become completely black.
                // ambientStrength controls how much background light reaches the object.
                // Example:ambientStrength = 0.15
                // means approximately 15% of the light color is used as constant ambient illumination.

                float ambientStrength = 0.3;

                // Multiply the light color by the ambient strength.
                // If:lightColor = (1, 1, 1) , ambientStrength = 0.15
                // then: ambient = (0.15, 0.15, 0.15)

                vec3 ambient = ambientStrength * lightColor;

                // PART 2: DIFFUSE LIGHTING
                // Diffuse lighting describes how strongly a surface is illuminated based on the angle between: surface normal &  direction toward the light
                // A surface facing the light receives strong illumination.
                // A surface facing away from the light receives little or no diffuse illumination.
                // Normalize the normal so its length becomes 1.
                // A normalized vector contains only directional information, which is important for the dot product.

                vec3 normal = normalize(fragNormal);

                // Calculate direction FROM the surface TO the light.
                // lightPos  = position of light
                // fragPos   = position of current surface point
                // Therefore: lightPos - fragPos gives the direction from the surface toward the light.

                vec3 lightDirection = normalize(lightPos - fragPos);

                // DOT PRODUCT
                // The dot product tells us how much the surface normal points toward the light.
                // Values:
                //     1.0  -> surface faces light directly
                //     0.0  -> surface is perpendicular to light
                //    <0.0  -> surface faces away from light
                // We use max(..., 0.0) because a surface facing away from the light should not receive negative lighting.

                float diffuseStrength =max(dot(normal, lightDirection),0.0);

                // Scale the light color according to the diffuse strength.
                // If diffuseStrength = 1: full light
                // If diffuseStrength = 0: no diffuse light

                vec3 diffuse = diffuseStrength * lightColor;

                // PART 3: SPECULAR LIGHTING
                // Specular lighting creates the shiny highlight that appears on a surface.
                // Example: A shiny metal/plastic cube has a bright spot where reflected light reaches the camera.
                // Calculate direction FROM the surface TO the camera.
                // viewPos  = camera position
                // fragPos  = current surface position
                // Therefore: viewPos - fragPos gives the direction toward the camera.

                vec3 viewDirection = normalize(viewPos - fragPos);

                // Calculate the direction in which the incoming light would be reflected from the surface.
                // lightDirection points: surface → light
                // But reflect() expects the incoming direction, so we use: -lightDirection
                // The normal determines how that incoming light bounces.

                vec3 reflectDirection =reflect(-lightDirection,normal);

                // Determine how closely the reflected light direction x points toward the camera.
                // If reflection points directly toward the camera: dot(viewDirection, reflectDirection) ≈ 1
                // If reflection points away: dot(...) ≈ 0 or negative
                // Again, negative values are clamped to zero.

                float specularStrength =pow(max(dot(viewDirection,reflectDirection),0.0),8.0);

                // SHININESS
                // The value 32.0 controls the size/sharpness of the specular highlight.
                // Lower value: wider / softer highlight
                // Higher value: smaller / sharper highlight
                // Example:
                //     8.0   -> broad highlight
                //     32.0  -> moderate highlight
                //     128.0 -> very sharp highlight

                vec3 specular = specularStrength * lightColor;

                // PART 4: COMBINE ALL LIGHTING COMPONENTS
                // We have three lighting components: Ambient, Diffuse, Specular
                // Add them together to get the total lighting affecting this fragment.
                //                     ┌── Ambient
                //                     │
                //                     ├── Diffuse
                // Total Lighting = ───┤
                //                     └── Specular

                vec3 lighting = ambient + diffuse + specular;

                // PART 5: APPLY THE OBJECT/FACE COLOR
                // 'lighting' tells us HOW MUCH light reaches the surface.
                // 'fragColor' tells us WHAT COLOR the surface is.
                // Multiplying them gives us the final lit object color.
                // Example:
                //     red face: fragColor = (1, 0, 0)
                //     white light: lighting = (0.8, 0.8, 0.8)
                //     result: (0.8, 0, 0)
                // So the red face becomes a darker/lighter red depending on the amount of light hitting it.

                vec3 result = lighting * fragColor;

                // PART 6: OUTPUT FINAL PIXEL COLOR
                // Convert the RGB result into RGBA.
                // result.x -> Red
                // result.y -> Green
                // result.z -> Blue
                // Alpha = 1.0 means fully opaque.
                // This is the final color that OpenGL writes for this fragment/pixel.

                finalColor = vec4(result, 1.0);
            }
        )";

    m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex,vertexShaderSource);//adding vertex shader
    m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,fragmentShaderSource);//adding fragment shader
    m_shaderProgram->link();//linking the shader
    Vertex vertices[] =
        {   //position + normal + colour
            //Each face has 2 triangles. So 6 points for each face. There are 6 faces, so 6x6=36 points.
            // FRONT - RED
            // Normal: +Z
            {{-1.0f, -1.0f,  1.0f},{ 0.0f, 0.0f, 1.0f},{ 1.0f, 0.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f},{ 0.0f, 0.0f, 1.0f},{ 1.0f, 0.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f},{ 0.0f, 0.0f, 1.0f},{ 1.0f, 0.0f, 0.0f}},

            {{ 1.0f,  1.0f,  1.0f},{ 0.0f, 0.0f, 1.0f},{ 1.0f, 0.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f},{ 0.0f, 0.0f, 1.0f},{ 1.0f, 0.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f},{ 0.0f, 0.0f, 1.0f},{ 1.0f, 0.0f, 0.0f}},

            // BACK - GREEN
            // Normal: -Z
            {{ 1.0f, -1.0f, -1.0f},{ 0.0f, 0.0f, -1.0f},{ 0.0f, 1.0f, 0.0f}},
            {{-1.0f, -1.0f, -1.0f},{ 0.0f, 0.0f, -1.0f},{ 0.0f, 1.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f},{ 0.0f, 0.0f, -1.0f},{ 0.0f, 1.0f, 0.0f}},

            {{-1.0f,  1.0f, -1.0f},{ 0.0f, 0.0f, -1.0f},{ 0.0f, 1.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f},{ 0.0f, 0.0f, -1.0f},{ 0.0f, 1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f},{ 0.0f, 0.0f, -1.0f},{ 0.0f, 1.0f, 0.0f}},

            // LEFT - BLUE
            // Normal: -X
            {{-1.0f, -1.0f, -1.0f},{-1.0f, 0.0f, 0.0f},{ 0.0f, 0.0f, 1.0f}},
            {{-1.0f, -1.0f,  1.0f},{-1.0f, 0.0f, 0.0f},{ 0.0f, 0.0f, 1.0f}},
            {{-1.0f,  1.0f,  1.0f},{-1.0f, 0.0f, 0.0f},{ 0.0f, 0.0f, 1.0f}},

            {{-1.0f,  1.0f,  1.0f},{-1.0f, 0.0f, 0.0f},{ 0.0f, 0.0f, 1.0f}},
            {{-1.0f,  1.0f, -1.0f},{-1.0f, 0.0f, 0.0f},{ 0.0f, 0.0f, 1.0f}},
            {{-1.0f, -1.0f, -1.0f},{-1.0f, 0.0f, 0.0f},{ 0.0f, 0.0f, 1.0f}},

            // RIGHT - YELLOW
            // Normal: +X
            {{ 1.0f, -1.0f,  1.0f},{ 1.0f, 0.0f, 0.0f},{ 1.0f, 1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f},{ 1.0f, 0.0f, 0.0f},{ 1.0f, 1.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f},{ 1.0f, 0.0f, 0.0f},{ 1.0f, 1.0f, 0.0f}},

            {{ 1.0f,  1.0f, -1.0f},{ 1.0f, 0.0f, 0.0f},{ 1.0f, 1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f},{ 1.0f, 0.0f, 0.0f},{ 1.0f, 1.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f},{ 1.0f, 0.0f, 0.0f},{ 1.0f, 1.0f, 0.0f}},

            // TOP - CYAN
            // Normal: +Y
            {{-1.0f,  1.0f,  1.0f},{ 0.0f, 1.0f, 0.0f},{ 0.0f, 1.0f, 1.0f}},
            {{ 1.0f,  1.0f,  1.0f},{ 0.0f, 1.0f, 0.0f},{ 0.0f, 1.0f, 1.0f}},
            {{ 1.0f,  1.0f, -1.0f},{ 0.0f, 1.0f, 0.0f},{ 0.0f, 1.0f, 1.0f}},

            {{ 1.0f,  1.0f, -1.0f},{ 0.0f, 1.0f, 0.0f},{ 0.0f, 1.0f, 1.0f}},
            {{-1.0f,  1.0f, -1.0f},{ 0.0f, 1.0f, 0.0f},{ 0.0f, 1.0f, 1.0f}},
            {{-1.0f,  1.0f,  1.0f},{ 0.0f, 1.0f, 0.0f},{ 0.0f, 1.0f, 1.0f}},

            // BOTTOM - MAGENTA
            // Normal: -Y
            {{-1.0f, -1.0f, -1.0f},{ 0.0f, -1.0f, 0.0f},{ 1.0f, 0.0f, 1.0f}},
            {{ 1.0f, -1.0f, -1.0f},{ 0.0f, -1.0f, 0.0f},{ 1.0f, 0.0f, 1.0f}},
            {{ 1.0f, -1.0f,  1.0f},{ 0.0f, -1.0f, 0.0f},{ 1.0f, 0.0f, 1.0f}},

            {{ 1.0f, -1.0f,  1.0f},{ 0.0f, -1.0f, 0.0f},{ 1.0f, 0.0f, 1.0f}},
            {{-1.0f, -1.0f,  1.0f},{ 0.0f, -1.0f, 0.0f},{ 1.0f, 0.0f, 1.0f}},
            {{-1.0f, -1.0f, -1.0f},{ 0.0f, -1.0f, 0.0f},{ 1.0f, 0.0f, 1.0f}}
        };
    m_vao.create();
    m_vao.bind();
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices,sizeof(vertices));//adding vertices info to vertex buffer
    //for position
    m_shaderProgram->enableAttributeArray(0);
    m_shaderProgram->setAttributeBuffer(0,GL_FLOAT,offsetof(Vertex, position),3,sizeof(Vertex));
    //for normal
    m_shaderProgram->enableAttributeArray(1);
    m_shaderProgram->setAttributeBuffer(1,GL_FLOAT,offsetof(Vertex, normal),3,sizeof(Vertex));
    //for colour
    m_shaderProgram->enableAttributeArray(2);
    m_shaderProgram->setAttributeBuffer(2,GL_FLOAT,offsetof(Vertex, color),3,sizeof(Vertex));
    m_vbo.release();
    m_vao.release();
}

void OpenGLWidget::resizeGL(int width,int height)
{
    if (height == 0)//safety check
        height = 1;
    m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);//setting aspect ratio
    m_projection.setToIdentity();//resetting projection matrix
    m_projection.perspective(45.0f,m_aspectRatio,0.1f,100.0f);
    /*
    45°       → field of view
    aspect    → width / height
    0.1       → near clipping plane
    100       → far clipping plane
    */
    glViewport(0,0,width,height);
}

void OpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // MODEL MATRIX
    QMatrix4x4 model;
    model.setToIdentity();
    model.rotate(m_rotationX,1.0f,0.0f,0.0f); //rotate with respect to X axis
    model.rotate(m_rotationY,0.0f,1.0f,0.0f); //rotate with respect to Y axis

    // VIEW MATRIX
    QMatrix4x4 view;
    view.setToIdentity();
    view.translate(0.0f,0.0f,-m_zoom);//the camera faces the negative z axis, that is why m_zoom is -ve. camera is moved for zooming

    // SHADER
    m_shaderProgram->bind();

    // Matrices
    m_shaderProgram->setUniformValue("model",model);
    m_shaderProgram->setUniformValue("view",view);
    m_shaderProgram->setUniformValue("projection",m_projection);

    // LIGHT
    m_shaderProgram->setUniformValue("lightPos",QVector3D(2.0f,2.0f,2.0f));
    m_shaderProgram->setUniformValue("lightColor",QVector3D(1.0f,1.0f,1.0f));

    // CAMERA POSITION
    m_shaderProgram->setUniformValue("viewPos",QVector3D(0.0f,0.0f,0.0f));

    // DRAW CUBE
    m_vao.bind();
    glDrawArrays(GL_TRIANGLES,0,36);
    m_vao.release();
    m_shaderProgram->release();
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mousePressed = true;
        m_lastMousePosition = event->position().toPoint();//getting the point where the mouse clicked
        event->accept();
    }
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_mousePressed)
        return;
    QPoint currentPosition = event->position().toPoint();//getting the point after the mouse is moved
    QPoint delta = currentPosition - m_lastMousePosition; //calculating the distance moved
    m_lastMousePosition = currentPosition; //now the lastMousePosition is currentPosition
    m_rotationY += delta.x() * 0.5f;//updating rotation values. here 0.5f is mouse sensitivity
    m_rotationX += delta.y() * 0.5f;
    update(); //calling paintGL
    event->accept();
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mousePressed = false;
        event->accept();
    }
}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
    int delta = event->angleDelta().y();
    if (delta > 0)
    {
        // Scroll up → zoom in(camera moves towards -z axis, so value must increase)
        m_zoom -= 0.5f; //0.5f is zoom increment level
    }
    else if (delta < 0)
    {
        // Scroll down → zoom out(camera moves away from -z axis, so value must decrease)
        m_zoom += 0.5f;
    }

    // Prevent getting too close
    if (m_zoom < 2.0f)
        m_zoom = 2.0f;

    // Prevent going too far away
    if (m_zoom > 15.0f)
        m_zoom = 15.0f;

    update();
    event->accept();
}