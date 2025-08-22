#include"menu.h"
#define NULL 0

static void toStr(char *buff, value_t val)
{
    value_t i;
    for (i = 0; i < DISP_WIDTH; i++)
        buff[i] = ' ';
    buff[i] = '\0';

    if (val == 0)
    {
        buff[--i] = '0';
    }
    else
    {
        while (val)
        {
            buff[--i] = '0' + (val % 10);
            val /= 10;
        }
    }
}

const char *MO_Repr(MObj *this)
{
    switch (this->type)
    {
    case COMBO:
    case MENU:
        return this->items[this->curr].text;
    case SLIDER:
        return this->repr_buff;
    }
    return NULL;
}

void MO_Left(MObj *this)
{
    switch (this->type)
    {
    case COMBO:
    case MENU:
        if (this->curr > 0)
            this->curr--;
        else
            this->curr = this->size - 1;
        break;
    case SLIDER:
        this->fake_value--;
        if (this->fake_value < this->min_bound || this->fake_value >= this->max_bound)
            this->fake_value = this->min_bound;
        toStr(this->repr_buff, this->fake_value);
        break;
    }
}

void MO_Right(MObj *this)
{
    switch (this->type)
    {
    case COMBO:
    case MENU:
        this->curr++;
        this->curr %= this->size;
        break;
    case SLIDER:
        this->fake_value++;
        if (this->fake_value > this->max_bound)
            this->fake_value = this->max_bound;
        toStr(this->repr_buff, this->fake_value);
        break;
    }
}

MObj *MO_Push(MObj *this)
{
    switch (this->type)
    {
    case MENU:
    {
        MObj *child = this->items[this->curr].child;
        if (child == NULL)
            return this;

        child->parent = this;
        if (child->type == SLIDER)
        {
            toStr(child->repr_buff, *child->true_value);
            child->fake_value = *child->true_value;
        }
        else if (child->type == COMBO)
        {
            for (value_t i = 0; i < child->size; i++)
            {
                if (child->items[i].value == *child->true_value)
                {
                    child->curr = i;
                    break;
                }
            }
        }
        return child;
    }
    case SLIDER:
        *this->true_value = this->fake_value;
        return this->parent;
    case COMBO:
        *this->true_value = this->items[this->curr].value;
        return this->parent;
    }

    return this;
}

MObj *MO_Back(MObj *this)
{
    switch (this->type)
    {
    case MENU:
        if (this->parent != NULL)
            return this->parent;
        return NULL;
        break;
    case SLIDER:
        return this->parent;
        break;
    case COMBO:
        return this->parent;
    }
    return this;
}