//
//  Rasterizer.cpp
//  CSE167 Spring 2015 Starter Code
//
//  Created by Ziyao Zhou on 10/13/15.
//  Copyright © 2015 RexWest. All rights reserved.
//
#include "Rasterizer.h"
#include "Window.h"
#include <iostream>
using namespace std;

int window_width = 512, window_height = 512;

static Matrix4 P;
static Matrix4 D;

static float* pixels = new float[window_width * window_height * 3];

static Drawable* toDraw;

static int frame=0;
static int t;
static int timebase=0;

static int mode = 0;

static float xmax,xmin,ymax,ymin;

static float *zbuffer = new float[window_width * window_height];

static int debugmode = -1;

void loadData()
{
    // put code to load data model here
}

// Clear frame buffer
void clearBuffer()
{
    Color clearColor = {0.0, 0.0, 0.0};   // clear color: black
    for (int i=0; i<window_width*window_height; ++i)
    {
        pixels[i*3]   = clearColor[0];
        pixels[i*3+1] = clearColor[1];
        pixels[i*3+2] = clearColor[2];
        
        zbuffer[i] = 1;
    }
}

void Rasterizer::resizePixelBuffer(int w, int h){
    pixels = new float[w * h * 3];
}

void Rasterizer::resizeZBuffer(int width, int height){
    zbuffer = new float[width * height];
}

void Rasterizer::draw(){
    glDrawPixels(window_width, window_height, GL_RGB, GL_FLOAT, pixels);
}

// Draw a point into the frame buffer
void drawPoint(int x, int y, float r, float g, float b)
{
    if (x < window_width && x >0 && y < window_height && y > 0) {
        int offset = y*window_width*3 + x*3;
        pixels[offset]   = r;
        pixels[offset+1] = g;
        pixels[offset+2] = b;
    }
}

void comparex(float a, float b, float c){
    if(a>b){
        xmax = a;
        xmin = b;
        if(c>a){
            xmax = c;
        }
        else{
            if(b>c){
                xmin = c;
            }
        }
    }
    else{
        xmax = b;
        xmin = a;
        if(c<a){
            xmin = c;
        }
        else{
            if(b<c){
                xmax = c;
            }
        }
    }
}

void comparey(float a, float b, float c){
    if(a>b){
        ymax = a;
        ymin = b;
        if(c>a){
            ymax = c;
        }
        else{
            if(b>c){
                ymin = c;
            }
        }
    }
    else{
        ymax = b;
        ymin = a;
        if(c<a){
            ymin = c;
        }
        else{
            if(b<c){
                ymax = c;
            }
        }
    }
}

float RandomFloat(float min, float max)
{
    float r = (float)rand() / (float)RAND_MAX;
    return min + r * (max - min);
}

void Rasterizer::rasterizeTriangle()
{
    int d = 0;
    float xa[3],ya[3];
    float za[3];
    
    Vector3 na[3];
    
    for (int i = 0; i<toDraw->getFSize(); i++) {
        Vector3 p3 = toDraw->facesGet(i);
        Vector3 n = toDraw->normalGet(i);
        
        Vector4 p = p3.toVector4(1.0);
   
        n = n.normalize();
        
        n.scale(1.0/2.0);
        n = n+Vector3(0.5,0.5,0.5);
        
        p = D*P*Globals::camera.getInverseMatrix()*toDraw->toWorld*p;
      
        p = p.dehomogenize();
 
        float x = (p[0]/p[3]);
        float y = (p[1]/p[3]);
        
        xa[d] = x;
        ya[d] = y;
        za[d] = p[2];
        na[d] = n;
        d++;
        
        if(d == 3){
            comparex(xa[0],xa[1],xa[2]);
            comparey(ya[0],ya[1],ya[2]);
            
            if (xmin<0)
                xmin = 0;
            if (xmax>window_width)
                xmax = window_width;
            if (ymin<0)
                ymin = 0;
            if (ymax>window_height)
                ymax = window_height;
            
            Color c;
            
            for (int i = xmin; i < (xmax+1); i++){
                for(int j = ymin; j < (ymax+1); j++){
                    
                    Vector3 v0(xa[2]-xa[0],ya[2]-ya[0],0.0);
                    Vector3 v1(xa[1]-xa[0],ya[1]-ya[0],0.0);
                    Vector3 v2(i-xa[0],j-ya[0],0.0);
                    
                    float dot00 = v0.dot(v0);
                    float dot01 = v0.dot(v1);
                    float dot02 = v0.dot(v2);
                    
                    float dot11 = v1.dot(v1);
                    float dot12 = v1.dot(v2);
                    
                    float invDenom = 1 /(dot00 * dot11 - dot01 * dot01);
                    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
                    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
                    
                    Vector3 n = na[0]*(1-u-v)+na[1]*v+na[2]*u;
                    
                    c = Color(n[0],n[1],n[2]);
                    
                    if ((u >= 0) && (u <= 1) && (v >= 0) && (v <= 1) && ((1 - u - v) >= 0) && ((1 - u - v) <= 1)){
                        float ztemp = za[0] * (1 - u - v) + za[1] * v + za[2] * u;
                        
                        if(zbuffer[i + window_width * j] >= ztemp || (zbuffer[i + window_width * j] <= 0 && ztemp >= 0)){
                            drawPoint(i, j, c[0], c[1], c[2]);
                            zbuffer[i + window_width * j] = ztemp;
                        }
                    }
                    
                    if (debugmode == 1) {
                        if (i == xmin || j == ymin || i == xmax || j == ymax) {
                            drawPoint(i, j, 1.0, 1.0, 1.0);
                        }
                    }
                   
                }
            }
            
            d = 0;
        }
    }
}
void Rasterizer::rasterizeTriangleZbuffer()
{
    int d = 0;
    float xa[3],ya[3];
    float za[3];
    
    for (int i = 0; i<toDraw->getFSize(); i++) {
        Vector3 p3 = toDraw->facesGet(i);
        Vector3 n = toDraw->normalGet(i);
        
        Vector4 p = p3.toVector4(1.0);
        
        p = D*P*Globals::camera.getInverseMatrix()*toDraw->toWorld*p;
        
        p = p.dehomogenize();
        
        float x = (int)(p[0]/p[3]+0.5);
        float y = (int)(p[1]/p[3]+0.5);
        
        xa[d] = x;
        ya[d] = y;
        za[d] = p[2];
        d++;
        
        if(d == 3){
            comparex(xa[0],xa[1],xa[2]);
            comparey(ya[0],ya[1],ya[2]);
            
            if (xmin<0)
                xmin = 0;
            if (xmax>window_width)
                xmax = window_width;
            if (ymin<0)
                ymin = 0;
            if (ymax>window_height)
                ymax = window_height;
            
            Color c(RandomFloat(0.0, 1.0),RandomFloat(0.0, 1.0),RandomFloat(0.0, 1.0));
            
            for (int i = xmin; i < (xmax+1); i++){
                for(int j = ymin; j < (ymax+1); j++){
                    
                    Vector3 v0(xa[2]-xa[0],ya[2]-ya[0],0.0);
                    Vector3 v1(xa[1]-xa[0],ya[1]-ya[0],0.0);
                    Vector3 v2(i-xa[0],j-ya[0],0.0);
                    
                    float dot00 = v0.dot(v0);
                    float dot01 = v0.dot(v1);
                    float dot02 = v0.dot(v2);
                    
                    float dot11 = v1.dot(v1);
                    float dot12 = v1.dot(v2);
                    
                    float invDenom = 1 /(dot00 * dot11 - dot01 * dot01);
                    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
                    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
                    
                    if ((u >= 0) && (u <= 1) && (v >= 0) && (v <= 1) && ((1 - u - v) >= 0) && ((1 - u - v) <= 1)){
                        float ztemp = za[0] * (1 - u - v) + za[1] * v + za[2] * u;
                        
                        if(zbuffer[i + window_width * j] > ztemp || ( ztemp > 0)){
                            
                            drawPoint(i, j, c[0], c[1], c[2]);
                            zbuffer[i + window_width * j] = ztemp;
                        }
                    }
                    
                    if (debugmode == 1) {
                        if (i == xmin || j == ymin || i == xmax || j == ymax) {
                            drawPoint(i, j, 1.0, 1.0, 1.0);
                        }
                    }
                    
                }
            }
            
            d = 0;
        }
    }
}
void Rasterizer::rasterizeTriangleRandom()
{
    int d = 0;
    float xa[3],ya[3];
    
    for (int i = 0; i<toDraw->getFSize(); i++) {
        Vector3 p3 = toDraw->facesGet(i);
        Vector3 n = toDraw->normalGet(i);
        
        Vector4 p = p3.toVector4(1.0);
        
        p = D*P*Globals::camera.getInverseMatrix()*toDraw->toWorld*p;
        
        p = p.dehomogenize();
        
        float x = p[0]/p[3];
        float y = p[1]/p[3];
        
        xa[d] = x;
        ya[d] = y;
        d++;
        
        if(d == 3){
            comparex(xa[0],xa[1],xa[2]);
            comparey(ya[0],ya[1],ya[2]);
            
            if (xmin<0)
                xmin = 0;
            if (xmax>window_width)
                xmax = window_width;
            if (ymin<0)
                ymin = 0;
            if (ymax>window_height)
                ymax = window_height;
            
            Color c(RandomFloat(0.0, 1.0),RandomFloat(0.0, 1.0),RandomFloat(0.0, 1.0));
            
            for (int i = xmin; i < (xmax+1); i++){
                for(int j = ymin; j < (ymax+1); j++){
                    
                    Vector3 v0(xa[2]-xa[0],ya[2]-ya[0],0.0);
                    Vector3 v1(xa[1]-xa[0],ya[1]-ya[0],0.0);
                    Vector3 v2(i-xa[0],j-ya[0],0.0);
                    
                    float dot00 = v0.dot(v0);
                    float dot01 = v0.dot(v1);
                    float dot02 = v0.dot(v2);
                    
                    float dot11 = v1.dot(v1);
                    float dot12 = v1.dot(v2);
                    
                    float invDenom = 1 /(dot00 * dot11 - dot01 * dot01);
                    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
                    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
                    
                    if ((u >= 0) && (u <= 1) && (v >= 0) && (v <= 1) && ((1 - u - v) >= 0) && ((1 - u - v) <= 1)){
                        drawPoint(i, j, c[0], c[1], c[2]);
                    }
                    
                    if (debugmode == 1) {
                        if (i == xmin || j == ymin || i == xmax || j == ymax) {
                            drawPoint(i, j, 1.0, 1.0, 1.0);
                        }
                    }
                    
                }
            }
            
            d = 0;
        }
    }
}

void rasterize()
{
    // Put your main rasterization loop here
    // It should go over the data model and call rasterizeTriangle for every triangle in it
    Rasterizer::rasterizeTriangle();
}

void Rasterizer::rasterizeVertex(){
    for (int i = 0; i<toDraw->getSize(); i++) {
        Vector3 p3 = toDraw->verticesGet(i);
        Vector4 p = p3.toVector4(1.0);
        
        p = D*P*Globals::camera.getInverseMatrix()*toDraw->toWorld*p;
        
        p.dehomogenize();
        
        int x = p[0]/p[3];
        int y = p[1]/p[3];
  
        if(x>0 && x<window_width && y>0 && y<window_height)
            drawPoint(x, y, 1.0, 1.0, 1.0);
    }
}

// Called whenever the window size changes
void Rasterizer::reshape(int new_width, int new_height)
{
    window_width  = new_width;
    window_height = new_height;
    delete[] pixels;
    delete[] zbuffer;
    P.makePerspectiveProjection(60.0, window_width, window_height, 1.0, 1000.0);
    
    D.makeViewport(0, window_width, 0, window_height);
    
    Rasterizer::resizePixelBuffer(window_width, window_height);
    
    Rasterizer::resizeZBuffer(window_width, window_height);
}

void house1() {
    Vector3 e = Vector3(0, 24.14, 24.14);
    Vector3 d = Vector3(0, 0, 0);
    Vector3 up = Vector3(0, 1, 0);
    
    Globals::camera.set(e, d, up);
    
    toDraw = &Globals::house;
}

void house2() {
    Vector3 e = Vector3( -28.33, 11.66, 23.33);
    Vector3 d = Vector3(-5, 0, 0);
    Vector3 up = Vector3(0, 1, 0.5);
    
    Globals::camera.set(e, d, up);
    
    toDraw = &Globals::house;
}

void Rasterizer::display()
{
    
    clearBuffer();
    if(toDraw != &Globals::house)
        Globals::camera = Camera();
    
    char s[50];
    
    frame++;
    t = glutGet(GLUT_ELAPSED_TIME);
    
    if (t - timebase > 1000) {
        sprintf(s,"FPS:%4.2f",frame*1000.0/(t-timebase));
        cout<<s<<endl;
        timebase = t;
        frame = 0;
    }
    
    if(mode == 0){
        Rasterizer::rasterizeTriangle();
    }
    if(mode == 1) {
        Rasterizer::rasterizeTriangleRandom();
    }
    if(mode == 2){
        Rasterizer::rasterizeTriangleZbuffer();
    }
    if(mode == 3){
        Rasterizer::rasterizeVertex();
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // glDrawPixels writes a block of pixels to the framebuffer
    glDrawPixels(window_width, window_height, GL_RGB, GL_FLOAT, pixels);
    
    glutSwapBuffers();
}

void Rasterizer::keyboard(unsigned char key, int, int)
{
    if (key == 'c') {
        Window::spinDirection *= -1;
    }
    if( key == 'X'){
        toDraw->translate(1,0);
    }
    if( key == 'x'){
        toDraw->translate(-1,0);
    }
    if( key == 'Y'){
        toDraw->translate(1,1);
    }
    if( key == 'y'){
        toDraw->translate(-1,1);
    }
    if( key == 'Z'){
        toDraw->translate(1,2);
    }
    if( key == 'z'){
        toDraw->translate(-1,2);
    }
    if( key == 'r'){
        toDraw->reset();
    }
    if( key == 'S'){
        toDraw->scale(1.25);
    }
    if( key == 's'){
        toDraw->scale(0.75);
    }
    if( key == 'O'){
        toDraw->orbit(5);
    }
    if( key == 'o'){
        toDraw->orbit(-5);
    }
    if( key == '+'){
        if (mode < 3) {
            mode ++;
        }
    }
    if( key == '-'){
        if (mode > 0) {
            mode --;
        }
    }
    if ( key == GLUT_KEY_F1 ){
        toDraw = &Globals::cube;
    }
    if ( key == GLUT_KEY_F2 ){
        house1();
    }
    if ( key == GLUT_KEY_F3 ){
        house2();
    }
    if ( key == GLUT_KEY_F4){
        toDraw = &Globals::bunny;
    }
    if ( key == GLUT_KEY_F5){
        toDraw = &Globals::dragon;
    }
    if ( key == GLUT_KEY_F6){
        toDraw = &Globals::bear;
    }
    if ( key == 'e') {
        Window::rasterize = -1;
    }
    if (key == 'd') {
        debugmode = -debugmode;
    }
    
    Rasterizer::display();
    
}
void Rasterizer::skeyboard(int key, int, int)
{
    if ( key == GLUT_KEY_F1 ){
        toDraw = &Globals::cube;
    }
    if ( key == GLUT_KEY_F2 ){
        Vector3 e = Vector3(0, 24.14, 24.14);
        Vector3 d = Vector3(0, 0, 0);
        Vector3 up = Vector3(0, 1, 0);
        
        Globals::camera.set(e, d, up);
        
        toDraw = &Globals::house;
    }
    if ( key == GLUT_KEY_F3 ){
        Vector3 e = Vector3( -28.33, 11.66, 23.33);
        Vector3 d = Vector3(-5, 0, 0);
        Vector3 up = Vector3(0, 1, 0.5);
        
        Globals::camera.set(e, d, up);
        
        toDraw = &Globals::house;
    }
    if ( key == GLUT_KEY_F4){
        toDraw = &Globals::bunny;
    }
    if ( key == GLUT_KEY_F5){
        toDraw = &Globals::dragon;
    }
    if ( key == GLUT_KEY_F6){
        toDraw = &Globals::bear;
    }
    
    Rasterizer::display();
    
}
void Rasterizer::init(int width, int height){
    window_height = height;
    window_width = width;
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    
    toDraw = &Globals::cube;
    
    P.makePerspectiveProjection(60.0, window_width, window_height, 1.0, 1000.0);
    D.makeViewport(0, window_width, 0, window_height);
    
    Rasterizer::display();
}
