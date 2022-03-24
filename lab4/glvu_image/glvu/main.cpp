//   Copyright © 2021, Renjie Chen @ USTC

#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#define FREEGLUT_STATIC
#include "gl_core_3_3.h"
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

#define TW_STATIC
#include <AntTweakBar.h>

#include <vector>
#include <string>
#include <iostream>

#include "glprogram.h"
#include "MyImage.h"
#include "VAOImage.h"
#include "VAOMesh.h"

using namespace std;

GLProgram MyMesh::prog;

MyMesh M;
int viewport[4] = {0, 0, 1280, 960};

bool showATB = true;

std::string imagefile = "boy.png";

MyImage img;
int resize_width, resize_height;

int mousePressButton;
int mouseButtonDown;
int mousePos[2];

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, viewport[2], viewport[3]);
    M.draw(viewport);

    if (showATB)
        TwDraw();
    glutSwapBuffers();
}

void onKeyboard(unsigned char code, int x, int y)
{
    if (!TwEventKeyboardGLUT(code, x, y))
    {
        switch (code)
        {
        case 17:
            exit(0);
        case 'f':
            glutFullScreenToggle();
            break;
        case ' ':
            showATB = !showATB;
            break;
        }
    }

    glutPostRedisplay();
}

void onMouseButton(int button, int updown, int x, int y)
{
    if (!showATB || !TwEventMouseButtonGLUT(button, updown, x, y))
    {
        mousePressButton = button;
        mouseButtonDown = updown;

        mousePos[0] = x;
        mousePos[1] = y;
    }

    glutPostRedisplay();
}

void onMouseMove(int x, int y)
{
    if (!showATB || !TwEventMouseMotionGLUT(x, y))
    {
        if (mouseButtonDown == GLUT_DOWN)
        {
            if (mousePressButton == GLUT_MIDDLE_BUTTON)
            {
                M.moveInScreen(mousePos[0], mousePos[1], x, y, viewport);
            }
        }
    }

    mousePos[0] = x;
    mousePos[1] = y;

    glutPostRedisplay();
}

void onMouseWheel(int wheel_number, int direction, int x, int y)
{
    if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
    {
    }
    else
        M.mMeshScale *= direction > 0 ? 1.1f : 0.9f;

    glutPostRedisplay();
}

int initGL(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowSize(960, 960);
    glutInitWindowPosition(200, 50);
    glutCreateWindow(argv[0]);

    // !Load the OpenGL functions. after the opengl context has been created
    if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
        return -1;

    glClearColor(1.f, 1.f, 1.f, 0.f);

    glutReshapeFunc([](int w, int h)
                    { viewport[2] = w; viewport[3] = h; TwWindowSize(w, h); });
    glutDisplayFunc(display);
    glutKeyboardFunc(onKeyboard);
    glutMouseFunc(onMouseButton);
    glutMotionFunc(onMouseMove);
    glutMouseWheelFunc(onMouseWheel);
    glutCloseFunc([]()
                  { exit(0); });
    return 0;
}

void uploadImage(const MyImage &img)
{
    int w = img.width();
    int h = img.height();
    float x[] = {0, 0, 0, w, 0, 0, w, h, 0, 0, h, 0};
    M.upload(x, 4, nullptr, 0, nullptr);

    M.tex.setImage(img);
    M.tex.setClamping(GL_CLAMP_TO_EDGE);
}

void readImage(const std::string &file)
{
    int w0 = img.width(), h0 = img.height();
    img = MyImage(file);
    uploadImage(img);
    resize_width = img.width();
    resize_height = img.height();

    if (w0 != img.width() || h0 != img.height())
        M.updateBBox();
}
/*
void imagePrint(MyImage &img)
{
    int w = img.width();
    int h = img.height();
    int comp = img.dim();
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            vector<BYTE> v = img(i, j);
            cout << "(" << i << "," << j << ")= ";
            for (int k = 0; k < 3; k++)
            {
                cout << (int)v[k] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }
}*/

//返回能量图的最小列
vector<int> get_col(MyImage &newimg, MyImage &saliency)
{
    int w_saliency = saliency.width();
    int h_saliency = saliency.height();
    vector<int> column;
    int min_sum = 214748364;
    column.clear();
    vector<int> temp;
    int last_j = 0;
    int sum = 0;
    for (int j = 0; j < w_saliency; j++)
    {
        last_j = j;
        temp.clear();
        sum = 0;
        for (int i = 0; i < h_saliency; i++)
        {
            if (i == 0)
            {
                last_j = j;
                temp.push_back(last_j);
                sum += saliency.getGrayValue(i, j);
            }
            else
            {
                //取三者的最小值
                int temp_j = last_j;
                int value = saliency.getGrayValue(i, temp_j);
                if (temp_j - 1 >= 0)
                {
                    int value_left = saliency.getGrayValue(i, temp_j - 1);
                    if (value > value_left)
                    {
                        value = value_left;
                        last_j = temp_j - 1;
                    }
                }
                if (temp_j + 1 < w_saliency)
                {
                    int value_right = saliency.getGrayValue(i, temp_j + 1);
                    if (value > value_right)
                    {
                        value = value_right;
                        last_j = temp_j + 1;
                    }
                }
                sum += value;
                temp.push_back(last_j);
            }
        }
        if (sum < min_sum)
        {
            min_sum = sum;
            column = temp;
        }
    }
    return column;
}

//返回图像的最小行
vector<int> get_line(MyImage &newimg, MyImage &saliency){
    int w_saliency = saliency.width();
    int h_saliency = saliency.height();
    vector<int> line;
    int min_sum = 214748364;
    line.clear();
    vector<int> temp;
    int last_i = 0;
    int sum = 0;
    for(int i=0;i<h_saliency;i++){
        last_i = i;
        temp.clear();
        sum = 0;
        for(int j=0;j<w_saliency;j++)
        {
            if(j==0)
            {
                last_i = i;
                temp.push_back(last_i);
                sum += saliency.getGrayValue(i,j);
            }
            else
            {
                //取三者的最小值
                int temp_i = last_i;
                int value = saliency.getGrayValue(temp_i,j);
                if(temp_i-1>=0)
                {
                    int value_up = saliency.getGrayValue(temp_i-1,j);
                    if(value>value_up)
                    {
                        value = value_up;
                        last_i = temp_i-1;
                    }
                }
                if(temp_i+1<h_saliency)
                {
                    int value_down = saliency.getGrayValue(temp_i+1,j);
                    if(value>value_down)
                    {
                        value = value_down;
                        last_i = temp_i+1;
                    }
                }
                sum += value;
                temp.push_back(last_i);
            }
        }
        if(sum<min_sum){
            min_sum = sum;
            line = temp;
        }
    }
    return line;
}



MyImage seamCarving(const MyImage &img, int w, int h)
{
    // TODO
    // return img.rescale(w, h);
    cout << "原图片尺寸为: width*height*comp=" << img.width() << "*" << img.height() << "*" << img.dim() << endl;
    cout << "缩放后的尺寸为: width*height*comp=" << w << "*" << h << "*" << img.dim() << endl;
    int w0 = img.width();
    int h0 = img.height();
    bool is_w_crop = (w < w0) ? true : false;
    bool is_h_crop = (h < h0) ? true : false;
    bool is_w_stretch = (w > w0) ? true : false;
    bool is_h_stretch = (h > h0) ? true : false;
    MyImage newimg = img;
    //
    string saliency_file = "boy_saliency.png";
    MyImage saliency = MyImage(saliency_file);
    int w_saliency = saliency.width();
    int h_saliency = saliency.height();
    //cout << "saliency的尺寸为: width*height*comp=" << w_saliency << "*" << h_saliency << "*" << saliency.dim() << endl;

    // width
    // imagePrint(newimg);
    vector<int> column;
    int loop_nums=0;
    if (is_w_crop)
    {
        //横向缩小图像loop_nums次
        loop_nums = w0 - w;
        for (int k = 0; k < loop_nums; k++)
        {
            column = get_col(newimg, saliency);
            saliency.crop_one_col(column);
            uploadImage(newimg);
            display();
            newimg.crop_one_col(column);
        }
    }
    else if (is_w_stretch)
    {
        //横向扩充图像loop_nums次
        loop_nums = w - w0;
        for (int k = 0; k < loop_nums; k++)
        {
            column = get_col(newimg, saliency);
            saliency.grayHaveVisitedOneCol(column);
            uploadImage(newimg);
            display();
            newimg.expand_one_col(column);
        }
        
    }
    // height
    vector<int> line;
    if (is_h_crop)
    {
        //纵向缩小图像loop_nums次
        loop_nums = h0 - h;
        for(int k=0;k< loop_nums; k++)
        {
            line = get_line(newimg, saliency);
            saliency.crop_one_line(line);
            uploadImage(newimg);
            display();
            newimg.crop_one_line(line);
        }
    }
    else if (is_h_stretch)
    {
        //纵向扩充图像loop_nums次
        loop_nums = h - h0;
        for(int k=0;k< loop_nums; k++)
        {
            line = get_line(newimg, saliency);
            saliency.grayHaveVisitedOneLine(line);
            uploadImage(newimg);
            display();
            newimg.expand_one_line(line);
        }
    }
    return newimg;
}

void createTweakbar()
{
    // Create a tweak bar
    TwBar *bar = TwNewBar("Image Viewer");
    TwDefine(" 'Image Viewer' size='220 150' color='0 128 255' text=dark alpha=128 position='5 5'"); // change default tweak bar size and color

    TwAddVarRW(bar, "Scale", TW_TYPE_FLOAT, &M.mMeshScale, " min=0 step=0.1");

    TwAddVarRW(bar, "Image filename", TW_TYPE_STDSTRING, &imagefile, " ");
    TwAddButton(
        bar, "Read Image", [](void *)
        { readImage(imagefile); },
        nullptr, "");

    TwAddVarRW(bar, "Resize Width", TW_TYPE_INT32, &resize_width, "group='Seam Carving' min=1 ");
    TwAddVarRW(bar, "Resize Height", TW_TYPE_INT32, &resize_height, "group='Seam Carving' min=1 ");
    TwAddButton(
        bar, "Run Seam Carving", [](void *img)
        {
        MyImage newimg = seamCarving(*(const MyImage*)img, resize_width, resize_height);
        uploadImage(newimg); },
        &img, "");
}

int main(int argc, char *argv[])
{
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), {100, 5000});

    if (initGL(argc, argv))
    {
        fprintf(stderr, "!Failed to initialize OpenGL!Exit...");
        exit(-1);
    }

    MyMesh::buildShaders();

    float x[] = {0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0};
    float uv[] = {0, 0, 1, 0, 1, 1, 0, 1};
    int t[] = {0, 1, 2, 2, 3, 0};

    M.upload(x, 4, t, 2, uv);

    //////////////////////////////////////////////////////////////////////////
    TwInit(TW_OPENGL_CORE, NULL);
    // Send 'glutGetModifers' function pointer to AntTweakBar;
    // required because the GLUT key event functions do not report key modifiers states.
    TwGLUTModifiersFunc(glutGetModifiers);
    glutSpecialFunc([](int key, int x, int y)
                    { TwEventSpecialGLUT(key, x, y); glutPostRedisplay(); }); // important for special keys like UP/DOWN/LEFT/RIGHT ...
    TwCopyStdStringToClientFunc([](std::string &dst, const std::string &src)
                                { dst = src; });

    createTweakbar();

    //////////////////////////////////////////////////////////////////////////
    atexit([]
           { TwDeleteAllBars();  TwTerminate(); }); // Called after glutMainLoop ends

    glutTimerFunc(
        1, [](int)
        { readImage(imagefile); },
        0);

    //////////////////////////////////////////////////////////////////////////
    glutMainLoop();

    return 0;
}
