//
//  House1.hpp
//  CSE167 Spring 2015 Starter Code
//
//  Created by Ziyao Zhou on 10/8/15.
//  Copyright © 2015 RexWest. All rights reserved.
//
#ifndef CSE167_House_h
#define CSE167_House_h

#include "Drawable.h"

class House : public Drawable
{

public:
    
    House();
    virtual void draw();
    
    long getSize();
    Vector3 verticesGet(int position);
    
    Vector3 normalGet(int position);
    
    Vector3 facesGet(int position);
    
    long getFSize(){
        return 60;
    }
    
    long getNSize(){
        return 42;
    }
    
    // scale
    void scale(float);
    
    // reset
    void reset(void);
    
    void translate(float distance, int option);
};

#endif
