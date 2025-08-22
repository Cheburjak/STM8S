#pragma once
#define DISP_WIDTH 16

typedef unsigned char value_t;

struct MObj;

typedef struct
{
    const char *text;
    union
    {
        struct MObj *child;
        value_t value;
    };
} MItem;

typedef struct MObj
{
    enum
    {
        MENU,
        SLIDER,
        COMBO
    } type;

    MItem *items;
    value_t size;
    value_t curr;
    value_t *true_value;
    value_t fake_value;
    value_t min_bound;
    value_t max_bound;

    struct MObj *parent;
    char repr_buff[DISP_WIDTH + 1];
} MObj;

const char *MO_Repr(MObj *this);
void MO_Left(MObj *this);
void MO_Right(MObj *this);
MObj *MO_Push(MObj *this);
MObj *MO_Back(MObj *this);
