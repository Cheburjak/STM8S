#include "menu.h"
#define NULL 0

static void toStr(char *buff, value_t val)
{
    value_t size = 0;
    for(value_t val_ = val ; val_ != 0; size++, val_/=10); 
   
    value_t idx;
    for (idx = 0; idx < DISP_WIDTH; idx++, buff[idx] = ' ');
    buff[idx] = '\0';
    buff[idx-1] = '>';
    buff[0] = '<';

    idx = idx - (idx-size) / 2;

    if (val == 0)
    {
        buff[idx] = '0';
    }
    else
    {
        while (val)
        {
            buff[--idx] = '0' + (val % 10);
            val /= 10;
        }
    }
}

const char *MO_Repr(MObj *this)
{
    switch (this->type)
    {
    case TCOMBO:
    case TMENU:
        return this->items[this->curr].text;
    case TSLIDER:
        return this->repr_buff;
    }
    return NULL;
}

const char *MO_Title(MObj *this, const char * Default)
{
    return this->parent ? MO_Repr(this->parent) : Default;
}

void MO_Left(MObj *this)
{
    switch (this->type)
    {
    case TCOMBO:
    case TMENU:
        if (this->curr > 0)
            this->curr--;
        else
            this->curr = this->size - 1;
        break;
    case TSLIDER:
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
    case TCOMBO:
    case TMENU:
        this->curr++;
        this->curr %= this->size;
        break;
    case TSLIDER:
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
    case TMENU:
    {
        MObj *child = this->items[this->curr].child;
        if (child == NULL)
            return this;

        child->parent = this;
        if (child->type == TSLIDER)
        {
            toStr(child->repr_buff, *child->true_value);
            child->fake_value = *child->true_value;
        }
        else if (child->type == TCOMBO)
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
    case TSLIDER:
        *this->true_value = this->fake_value;
        return this->parent;
    case TCOMBO:
        *this->true_value = this->items[this->curr].value;
        return this->parent;
    }

    return this;
}

MObj *MO_Back(MObj *this)
{
    switch (this->type)
    {
    case TMENU:
        if (this->parent != NULL)
            return this->parent;
        return NULL;
        break;
    case TSLIDER:
        return this->parent;
        break;
    case TCOMBO:
        return this->parent;
    }
    return this;
}